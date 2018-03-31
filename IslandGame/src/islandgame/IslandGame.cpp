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
#include <host/ConnectedClient.h>
#include <host/HostWorld.h>

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

	// Load data files.
	IslandGame::IslandGameData gameData;
	gameData.LoadBehaviourTreesInDirectory(dataDirectory / k_behaviourTreesPath);
	gameData.LoadActorInfosInDirectory(dataDirectory / k_actorInfosPath);

	// Create and run a host and client.
	constexpr size_t k_messageCapacity = 256;
	// TODO std::function will not work when actually performing this over a network
	Collection::LocklessQueue<std::function<void()>> clientToHostMessages{ k_messageCapacity };
	Collection::LocklessQueue<std::function<void()>> hostToClientMessages{ k_messageCapacity };

	Host::HostWorld::HostFactory hostFactory = [&]()
	{
		return Mem::MakeUnique<IslandGame::Host::IslandGameHost>(gameData);
	};
	Client::ClientWorld::ClientFactory clientFactory = [](Client::ConnectedHost& conenctedHost)
	{
		return Mem::MakeUnique<IslandGame::Client::IslandGameClient>(conenctedHost);
	};

	Host::HostWorld hostWorld{ clientToHostMessages, std::move(hostFactory) };
	Client::ClientWorld clientWorld{ hostToClientMessages, std::move(clientFactory) };

	// Connect the client and the host.
	class LocalConnectedClient : public Host::ConnectedClient
	{
		Client::ClientWorld& m_clientWorld;
		Collection::LocklessQueue<std::function<void()>>& m_hostToClientMessages;

	public:
		LocalConnectedClient(Client::ClientWorld& clientWorld,
			Collection::LocklessQueue<std::function<void()>>& hostToClientMessages)
			: m_clientWorld(clientWorld)
			, m_hostToClientMessages(hostToClientMessages)
		{
			m_clientID = 1;
		}

		void NotifyOfHostDisconnected() override
		{
			Client::ClientWorld& clientWorld = m_clientWorld;
			m_hostToClientMessages.TryPush([&clientWorld]() { clientWorld.NotifyOfHostDisconnected(); });
		}
	};

	class LocalConnectedHost : public Client::ConnectedHost
	{
		Host::HostWorld& m_hostWorld;
		Collection::LocklessQueue<std::function<void()>>& m_clientToHostMessages;

	public:
		LocalConnectedHost(Host::HostWorld& hostWorld,
			Collection::LocklessQueue<std::function<void()>>& clientToHostMessages)
			: m_hostWorld(hostWorld)
			, m_clientToHostMessages(clientToHostMessages)
		{
			m_clientID = 1;
		}

		void Disconnect() override
		{
			Host::HostWorld& hostWorld = m_hostWorld;
			uint16_t clientID = m_clientID;
			m_clientToHostMessages.TryPush([&hostWorld, clientID]() { hostWorld.NotifyOfClientDisconnected(clientID); });
		}
	};

	hostWorld.NotifyOfClientConnected(Mem::MakeUnique<LocalConnectedClient>(clientWorld, hostToClientMessages));
	clientWorld.NotifyOfHostConnected(Mem::MakeUnique<LocalConnectedHost>(hostWorld, clientToHostMessages));

	// Run the HostWorld until the client terminates.
	while (hostWorld.GetNumConnectedClients() > 0)
	{
		std::this_thread::yield();
	}

	return 0;
}
