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
}

int main(const int argc, const char* argv[])
{
	using namespace Internal_IslandGame;

	// Extract the data directory from the command line parameters.
	const auto params = Collection::ProgramParameters(argc, argv);
	std::string dataDirectoryStr;
	if (!params.TryGet(k_dataDirectoryParameter, dataDirectoryStr))
	{
		std::cerr << "Missing required parameter: -datapath <dir>" << std::endl;
		return -1;
	}

	const File::Path dataDirectory = File::MakePath(dataDirectoryStr.c_str());

	// Find the vertex and fragment shaders in the data directory.
	const File::Path vertexShaderFile = dataDirectory / k_vertexShaderPath;
	const File::Path fragmentShaderFile = dataDirectory / k_fragmentShaderPath;
	
	// Create a render instance. Because a render instance creates a window,
	// it must be created and managed on the main thread.
	constexpr size_t k_messageCapacity = 256;
	Collection::LocklessQueue<Client::InputMessage> inputToClientMessages{ k_messageCapacity };
	Collection::LocklessQueue<Client::MessageToRenderInstance> clientToRenderInstanceMessages{ k_messageCapacity };
	
	Mem::UniquePtr<Client::IRenderInstance> renderInstance = Mem::MakeUnique<VulkanRenderer::VulkanInstance>(
		clientToRenderInstanceMessages, inputToClientMessages, "IslandGame", vertexShaderFile, fragmentShaderFile);

	// Load data files.
	IslandGame::IslandGameData gameData;
	gameData.LoadBehaviourTreesInDirectory(dataDirectory / k_behaviourTreesPath);
	gameData.LoadActorInfosInDirectory(dataDirectory / k_actorInfosPath);

	// Create and run a host and client.
	Collection::LocklessQueue<Client::MessageToHost> clientToHostMessages{ k_messageCapacity };
	Collection::LocklessQueue<Host::MessageToClient> hostToClientMessages{ k_messageCapacity };

	Host::HostWorld::HostFactory hostFactory = [&]()
	{
		return Mem::MakeUnique<IslandGame::Host::IslandGameHost>(gameData);
	};
	Client::ClientWorld::ClientFactory clientFactory = [](Client::ConnectedHost& conenctedHost)
	{
		return Mem::MakeUnique<IslandGame::Client::IslandGameClient>(conenctedHost);
	};

	Host::HostWorld hostWorld{ clientToHostMessages, std::move(hostFactory) };
	Client::ClientWorld clientWorld{ inputToClientMessages, hostToClientMessages, std::move(clientFactory) };

	// Connect the client and the host.
	constexpr uint16_t clientID = 1;
	hostWorld.NotifyOfClientConnected(Mem::MakeUnique<Host::ConnectedClient>(clientID, hostToClientMessages));
	clientWorld.NotifyOfHostConnected(Mem::MakeUnique<Client::ConnectedHost>(clientID, clientToHostMessages));

	// Run the window while the client and host run in other threads. Stop when the host stops.
	while (renderInstance->Update() == Client::IRenderInstance::Status::Running
		&& hostWorld.GetNumConnectedClients() > 0)
	{
		std::this_thread::yield();
	}

	return 0;
}
