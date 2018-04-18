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
#include <conductor/ApplicationErrorCode.h>
#include <conductor/IGameData.h>
#include <conductor/LocalClientHostMain.h>
#include <conductor/RemoteClientMain.h>
#include <host/ConnectedClient.h>
#include <host/HostNetworkWorld.h>
#include <host/HostWorld.h>
#include <host/MessageToClient.h>
#include <network/Socket.h>

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

enum class ApplicationMode
{
	Invalid = 0,
	Client,
	Host,
};

int ClientMain(const Collection::ProgramParameters& params, const File::Path& dataDirectory, std::string& hostParam);
int HostMain(const Collection::ProgramParameters& params, const File::Path& dataDirectory,
	const std::string& port);
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
		return static_cast<int>(Conductor::ApplicationErrorCode::MissingDatapath);
	}

	const File::Path dataDirectory = File::MakePath(dataDirectoryStr.c_str());

	// Determine the application mode from the command line parameters.
	ApplicationMode applicationMode = ApplicationMode::Invalid;
	std::string applicationModeParamater;
	if (params.TryGet(k_applicationModeClientParameter, applicationModeParamater))
	{
		applicationMode = ApplicationMode::Client;
	}
	else if (params.TryGet(k_applicationModeHostParameter, applicationModeParamater))
	{
		applicationMode = ApplicationMode::Host;
	}
	else
	{
		std::cerr << "Missing application mode parameter: -client hostName or -host" << std::endl;
		return static_cast<int>(Conductor::ApplicationErrorCode::MissingApplicationMode);
	}

	// Run the application in the specified mode.
	if (applicationMode == ApplicationMode::Client)
	{
		return ClientMain(params, dataDirectory, applicationModeParamater);
	}
	return HostMain(params, dataDirectory, applicationModeParamater);
}

int Internal_IslandGame::ClientMain(
	const Collection::ProgramParameters& params,
	const File::Path& dataDirectory,
	std::string& hostParam)
{
	// Ensure a host parameter was specified.
	if (hostParam.size() < 3)
	{
		return static_cast<int>(Conductor::ApplicationErrorCode::MissingClientHostName);
	}

	// Define the factory functions that abstract game code away from engine code.
	Client::RenderInstanceFactory renderInstanceFactory = [](const File::Path& dataDirectory,
		Collection::LocklessQueue<Client::MessageToRenderInstance>& clientToRenderInstanceMessages,
		Collection::LocklessQueue<Client::InputMessage>& inputToClientMessages)
	{
		const File::Path vertexShaderFile = dataDirectory / k_vertexShaderPath;
		const File::Path fragmentShaderFile = dataDirectory / k_fragmentShaderPath;
	
		return Mem::MakeUnique<VulkanRenderer::VulkanInstance>(
			clientToRenderInstanceMessages, inputToClientMessages, "IslandGame", vertexShaderFile, fragmentShaderFile);
	};

	Conductor::GameDataFactory gameDataFactory = [](const File::Path& dataDirectory)
	{
		auto gameData = Mem::MakeUnique<IslandGame::IslandGameData>();
		gameData->LoadBehaviourTreesInDirectory(dataDirectory / k_behaviourTreesPath);
		gameData->LoadActorInfosInDirectory(dataDirectory / k_actorInfosPath);
		return gameData;
	};

	Client::ClientWorld::ClientFactory clientFactory =
		[](const Conductor::IGameData& gameData, Client::ConnectedHost& connectedHost)
	{
		return Mem::MakeUnique<IslandGame::Client::IslandGameClient>(connectedHost);
	};

	Host::HostWorld::HostFactory hostFactory = [](const Conductor::IGameData& gameData)
	{
		return Mem::MakeUnique<IslandGame::Host::IslandGameHost>(static_cast<const IslandGame::IslandGameData&>(gameData));
	};

	// If the host is specified as "newhost", spin up a local host with no network thread: just direct communication.
	// Otherwise, connect to a remote host.
	if (strcmp(hostParam.c_str(), "newhost") == 0)
	{
		const Conductor::ApplicationErrorCode errorCode = Conductor::LocalClientHostMain(params, dataDirectory,
			std::move(renderInstanceFactory), std::move(gameDataFactory), std::move(clientFactory), std::move(hostFactory));
		return static_cast<int>(errorCode);
	}
	else
	{
		// Extract the host name and port from hostParam.
		const size_t portStartIndex = hostParam.find_last_of(':');
		if (portStartIndex == std::string::npos)
		{
			return static_cast<int>(Conductor::ApplicationErrorCode::MissingClientHostPort);
		}
		hostParam[portStartIndex] = '\0';
		const char* const hostName = hostParam.c_str();
		const char* const hostPort = hostName + portStartIndex + 1;

		const Conductor::ApplicationErrorCode errorCode = Conductor::RemoteClientMain(params, dataDirectory,
			hostName, hostPort,std::move(renderInstanceFactory), std::move(gameDataFactory), std::move(clientFactory));
		return static_cast<int>(errorCode);
	}

	return static_cast<int>(Conductor::ApplicationErrorCode::NoError);
}

int Internal_IslandGame::HostMain(const Collection::ProgramParameters& params, const File::Path& dataDirectory,
	const std::string& port)
{
	// Ensure a port was specified.
	if (port.empty())
	{
		return static_cast<int>(Conductor::ApplicationErrorCode::MissingHostPort);
	}

	// Load data files.
	IslandGame::IslandGameData gameData;
	gameData.LoadBehaviourTreesInDirectory(dataDirectory / k_behaviourTreesPath);
	gameData.LoadActorInfosInDirectory(dataDirectory / k_actorInfosPath);

	// Initialize the network socket API.
	if (!Network::TryInitializeSocketAPI())
	{
		return static_cast<int>(Conductor::ApplicationErrorCode::FailedToInitializeSocketAPI);
	}

	// Setup the network world and verify it is running.
	Host::HostNetworkWorld hostNetworkWorld{ port.c_str() };
	if (!hostNetworkWorld.IsRunning())
	{
		Network::ShutdownSocketAPI();
		return static_cast<int>(Conductor::ApplicationErrorCode::FailedToInitializeNetworkThread);
	}

	// Create and run a host.
	Host::HostWorld::HostFactory hostFactory = [](const Conductor::IGameData& gameData)
	{
		return Mem::MakeUnique<IslandGame::Host::IslandGameHost>(static_cast<const IslandGame::IslandGameData&>(gameData));
	};
	Host::HostWorld hostWorld{ gameData, hostNetworkWorld.GetClientToHostMessageQueue(), std::move(hostFactory) };
	
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

	// Shutdown the socket API.
	Network::ShutdownSocketAPI();
	
	return static_cast<int>(Conductor::ApplicationErrorCode::NoError);
}
