#include <islandgame/IslandGameData.h>

#include <islandgame/client/IslandGameClient.h>
#include <islandgame/host/IslandGameHost.h>

#include <asset/AssetManager.h>

#include <collection/LocklessQueue.h>
#include <collection/ProgramParameters.h>

#include <file/Path.h>

#include <client/ClientWorld.h>
#include <client/ConnectedHost.h>
#include <client/IRenderInstance.h>
#include <client/MessageToHost.h>
#include <client/MessageToRenderInstance.h>
#include <conductor/ApplicationErrorCode.h>
#include <conductor/IGameData.h>
#include <conductor/LocalClientHostMain.h>
#include <conductor/HostMain.h>
#include <conductor/RemoteClientMain.h>
#include <host/ConnectedClient.h>
#include <host/HostNetworkWorld.h>
#include <host/HostWorld.h>
#include <host/MessageToClient.h>
#include <input/InputMessage.h>
#include <network/Socket.h>

#include <renderer/AssetTypes.h>
#include <renderer/RenderInstance.h>

#include <iostream>

namespace Internal_IslandGame
{
constexpr char* k_dataDirectoryParameter = "-datapath";
constexpr char* k_userGamePath = "Documents/My Games/IslandGame/";

constexpr char* k_vertexShaderPath = "shaders/vertex_shader.glsl";
constexpr char* k_fragmentShaderPath = "shaders/fragment_shader.glsl";

constexpr char* k_behaviourTreesPath = "behaviour_trees";
constexpr char* k_entityInfosPath = "entity_infos";

constexpr char* k_applicationModeClientParameter = "-client";
constexpr char* k_applicationModeHostParameter = "-host";

enum class ApplicationMode
{
	Invalid = 0,
	Client,
	Host,
};

int ClientMain(const Collection::ProgramParameters& params, const File::Path& dataDirectory,
	const File::Path& userDirectory, Asset::AssetManager& assetManager, std::string& hostParam);
int HostMain(const Collection::ProgramParameters& params, const File::Path& dataDirectory,
	const File::Path& userDirectory, Asset::AssetManager& assetManager, const std::string& port);

// Define the factory functions that abstract game code away from engine code.
Client::RenderInstanceFactory MakeRenderInstanceFactory()
{
	return [](Asset::AssetManager& assetManager,
		const File::Path& dataDirectory,
		Collection::LocklessQueue<Client::MessageToRenderInstance>& clientToRenderInstanceMessages,
		Collection::LocklessQueue<Input::InputMessage>& inputToClientMessages)
	{
		return Mem::MakeUnique<Renderer::RenderInstance>(assetManager,
			clientToRenderInstanceMessages, inputToClientMessages, "IslandGame");
	};
}

Conductor::GameDataFactory MakeGameDataFactory()
{
	return [](Asset::AssetManager& assetManager, const File::Path& dataDirectory, const File::Path& userDirectory)
	{
		auto gameData = Mem::MakeUnique<IslandGame::IslandGameData>(dataDirectory, userDirectory, assetManager);
		Renderer::RenderInstance::RegisterComponentTypes(gameData->GetComponentReflector(), 
			gameData->GetComponentInfoFactory());

		gameData->LoadBehaviourTreesInDirectory(dataDirectory / k_behaviourTreesPath);
		gameData->LoadEntityInfosInDirectory(dataDirectory / k_entityInfosPath);
		return gameData;
	};
}

Client::ClientWorld::ClientFactory MakeClientFactory()
{
	return [](const Conductor::IGameData& gameData, Client::ConnectedHost& connectedHost)
	{
		return Mem::MakeUnique<IslandGame::Client::IslandGameClient>(
			static_cast<const IslandGame::IslandGameData&>(gameData), connectedHost);
	};
}

Host::HostWorld::HostFactory MakeHostFactory()
{
	return [](const Conductor::IGameData& gameData)
	{
		return Mem::MakeUnique<IslandGame::Host::IslandGameHost>(
			static_cast<const IslandGame::IslandGameData&>(gameData));
	};
}
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
		return static_cast<int>(Conductor::ApplicationErrorCode::MissingDataPath);
	}

	const File::Path dataDirectory = File::MakePath(dataDirectoryStr.c_str());

	// Get the user directory.
	char userDirBuffer[128];
	size_t userDirLength;
	const errno_t userDirErrorCode = getenv_s(&userDirLength, userDirBuffer, "USERPROFILE");

	if (userDirErrorCode != 0 || userDirLength == 0)
	{
		std::cerr << "Failed to get the user path from the system." << std::endl;
		return static_cast<int>(Conductor::ApplicationErrorCode::FailedToGetUserPath);
	}

	const File::Path userDirectory = File::MakePath(userDirBuffer) / k_userGamePath;

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

	// Create an asset manager and register the asset types it needs.
	Asset::AssetManager assetManager{ dataDirectory };
	Renderer::RegisterAssetTypes(assetManager);

	// Run the application in the specified mode.
	const int result = (applicationMode == ApplicationMode::Client)
		? ClientMain(params, dataDirectory, userDirectory, assetManager, applicationModeParamater)
		: HostMain(params, dataDirectory, userDirectory, assetManager, applicationModeParamater);

	// Unregister the asset types from the asset manager in the opposite order they were registered.
	Renderer::UnregisterAssetTypes(assetManager);

	return result;
}

int Internal_IslandGame::ClientMain(
	const Collection::ProgramParameters& params,
	const File::Path& dataDirectory,
	const File::Path& userDirectory,
	Asset::AssetManager& assetManager,
	std::string& hostParam)
{
	// Ensure a host parameter was specified.
	if (hostParam.size() < 3)
	{
		return static_cast<int>(Conductor::ApplicationErrorCode::MissingClientHostName);
	}

	// If the host is specified as "newhost", spin up a local host with no network thread: just direct communication.
	// Otherwise, connect to a remote host.
	if (strcmp(hostParam.c_str(), "newhost") == 0)
	{
		const Conductor::ApplicationErrorCode errorCode = Conductor::LocalClientHostMain(params, dataDirectory, userDirectory,
			assetManager, MakeRenderInstanceFactory(), MakeGameDataFactory(), MakeClientFactory(), MakeHostFactory());
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

		const Conductor::ApplicationErrorCode errorCode = Conductor::RemoteClientMain(params, dataDirectory, userDirectory,
			assetManager, hostName, hostPort, MakeRenderInstanceFactory(), MakeGameDataFactory(), MakeClientFactory());
		return static_cast<int>(errorCode);
	}

	return static_cast<int>(Conductor::ApplicationErrorCode::NoError);
}

int Internal_IslandGame::HostMain(
	const Collection::ProgramParameters& params,
	const File::Path& dataDirectory,
	const File::Path& userDirectory,
	Asset::AssetManager& assetManager,
	const std::string& port)
{
	// Ensure a port was specified.
	if (port.empty())
	{
		return static_cast<int>(Conductor::ApplicationErrorCode::MissingHostPort);
	}
	
	// Run the host.
	const Conductor::ApplicationErrorCode errorCode = Conductor::HostMain(params, dataDirectory, userDirectory,
		assetManager, port.c_str(), MakeGameDataFactory(), MakeHostFactory());
	return static_cast<int>(errorCode);
}
