#include <islandgame/IslandGameData.h>

#include <islandgame/client/IslandGameClient.h>
#include <islandgame/host/IslandGameHost.h>

#include <behave/ActorInfoManager.h>
#include <behave/ActorManager.h>
#include <behave/BehaveContext.h>
#include <behave/BehaviourSystem.h>
#include <behave/components/SceneTransformComponent.h>
#include <behave/components/SceneTransformComponentInfo.h>

#include <collection/LocklessQueue.h>
#include <collection/ProgramParameters.h>

#include <file/Path.h>

#include <client/ClientWorld.h>
#include <client/ConnectedHost.h>
#include <client/InputMessage.h>
#include <client/IRenderInstance.h>
#include <client/MessageToHost.h>
#include <client/MessageToRenderInstance.h>
#include <host/ConnectedClient.h>
#include <host/HostNetworkWorld.h>
#include <host/HostWorld.h>
#include <host/MessageToClient.h>

#include <vulkanrenderer/VulkanInstance.h>

#include <iostream>

namespace Internal_IslandGame
{
constexpr char* k_dataDirectoryParameter = "-datapath";

constexpr char* k_vertexShaderPath = "shaders/vertex_shader.glsl";
constexpr char* k_fragmentShaderPath = "shaders/fragment_shader.glsl";

constexpr char* k_behaviourTreesPath = "behaviour_trees";
constexpr char* k_actorInfosPath = "actor_infos";

constexpr char* k_applicationModeClientParameter = "-client";
constexpr char* k_applicationModeHostParameter = "-host";

int ClientMain(const Collection::ProgramParameters& params, const File::Path& dataDirectory, const std::string& hostName);
int HostMain(const Collection::ProgramParameters& params, const File::Path& dataDirectory);
}

int main(const int argc, const char* argv[])
{
	using namespace Internal_IslandGame;

	// Extract the data directory from the command line parameters.
	const Collection::ProgramParameters params{ argc, argv };
	std::string dataDirectoryStr;
	if (!params.TryGet(k_dataDirectoryParameter, dataDirectoryStr))
	{
		std::cerr << "Missing required parameter: -datapath <dir>" << std::endl;
		return -1;
	}

	const File::Path dataDirectory = File::MakePath(dataDirectoryStr.c_str());

	// Determine the application mode from the command line parameters.
	std::string applicationModeParamater;
	if (params.TryGet(k_applicationModeClientParameter, applicationModeParamater))
	{
		return ClientMain(params, dataDirectory, applicationModeParamater);
	}
	if (params.TryGet(k_applicationModeHostParameter, applicationModeParamater))
	{
		return HostMain(params, dataDirectory);
	}

	std::cerr << "Missing application mode parameter: -client hostName or -host" << std::endl;
	return -1;
}

int Internal_IslandGame::ClientMain(const Collection::ProgramParameters& params, const File::Path& dataDirectory,
	const std::string& hostName)
{
	// Find the vertex and fragment shaders in the data directory.
	const File::Path vertexShaderFile = dataDirectory / k_vertexShaderPath;
	const File::Path fragmentShaderFile = dataDirectory / k_fragmentShaderPath;
	
	// Create a render instance. Because a render instance creates a window,
	// it must be created and managed on the main thread.
	constexpr size_t k_clientRenderMessageCapacity = 256;
	Collection::LocklessQueue<Client::InputMessage> inputToClientMessages{ k_clientRenderMessageCapacity };
	Collection::LocklessQueue<Client::MessageToRenderInstance> clientToRenderInstanceMessages{
		k_clientRenderMessageCapacity };
	
	Mem::UniquePtr<Client::IRenderInstance> renderInstance = Mem::MakeUnique<VulkanRenderer::VulkanInstance>(
		clientToRenderInstanceMessages, inputToClientMessages, "IslandGame", vertexShaderFile, fragmentShaderFile);

	// Load data files.
	IslandGame::IslandGameData gameData;
	gameData.LoadBehaviourTreesInDirectory(dataDirectory / k_behaviourTreesPath);
	gameData.LoadActorInfosInDirectory(dataDirectory / k_actorInfosPath);

	// Create the message queues that will allow the client and host to communicate.
	Collection::LocklessQueue<Client::MessageToHost> clientToHostMessages{
		Host::HostNetworkWorld::k_inboundMessageCapacity };
	Collection::LocklessQueue<Host::MessageToClient> hostToClientMessages{
		Host::HostNetworkWorld::k_outboundMessageCapacityPerClient };

	// Create the client and connect it to the specified host.
	Client::ClientWorld::ClientFactory clientFactory = [](Client::ConnectedHost& conenctedHost)
	{
		return Mem::MakeUnique<IslandGame::Client::IslandGameClient>(conenctedHost);
	};
	Client::ClientWorld clientWorld{ inputToClientMessages, hostToClientMessages, std::move(clientFactory) };

	if (strcmp(hostName.c_str(), "newhost") == 0)
	{
		// Connect the client to a new host.
		Host::HostWorld::HostFactory hostFactory = [&]()
		{
			return Mem::MakeUnique<IslandGame::Host::IslandGameHost>(gameData);
		};
		Host::HostWorld hostWorld{ clientToHostMessages, std::move(hostFactory) };
		
		constexpr Client::ClientID clientID = Host::HostNetworkWorld::k_localClientID;
		hostWorld.NotifyOfClientConnected(Mem::MakeUnique<Host::ConnectedClient>(clientID, hostToClientMessages));
		clientWorld.NotifyOfHostConnected(Mem::MakeUnique<Client::ConnectedHost>(clientID, clientToHostMessages));

		// Run the window while the client and host run in other threads. Stop when the host stops.
		while (renderInstance->Update() == Client::IRenderInstance::Status::Running
			&& hostWorld.GetNumConnectedClients() > 0)
		{
			std::this_thread::yield();
		}
	}
	else
	{
		// TODO Connect the client to a networked host.
	}

	return 0;
}

int Internal_IslandGame::HostMain(const Collection::ProgramParameters& params, const File::Path& dataDirectory)
{
	// Load data files.
	IslandGame::IslandGameData gameData;
	gameData.LoadBehaviourTreesInDirectory(dataDirectory / k_behaviourTreesPath);
	gameData.LoadActorInfosInDirectory(dataDirectory / k_actorInfosPath);

	// Create and run a host and a network thread.
	Host::HostNetworkWorld hostNetworkWorld;

	Host::HostWorld::HostFactory hostFactory = [&]()
	{
		return Mem::MakeUnique<IslandGame::Host::IslandGameHost>(gameData);
	};
	Host::HostWorld hostWorld{ hostNetworkWorld.GetClientToHostMessageQueue(), std::move(hostFactory) };
	
	// Create a thread that processes console input for as long as the network thread is running.
	std::thread consoleInputThread{ [&hostNetworkWorld]()
	{
		while (hostNetworkWorld.IsRunning())
		{
			std::string consoleInput;
			std::getline(std::cin, consoleInput);
			if (!consoleInput.empty())
			{
				hostNetworkWorld.NotifyOfConsoleInput(std::move(consoleInput));
			}
			std::this_thread::yield();
		}
	}};
	consoleInputThread.detach();

	// Block this thread until the network thread stops.
	hostNetworkWorld.WaitForShutdown();

	return 0;
}
