#include <asset/AssetManager.h>
#include <client/ClientWorld.h>
#include <client/ClientNetworkWorld.h>
#include <client/ConnectedHost.h>
#include <conductor/GameData.h>
#include <host/ConnectedClient.h>
#include <host/HostWorld.h>
#include <host/HostNetworkWorld.h>
#include <input/InputMessage.h>

#define DLL_EXPORT __declspec(dllexport)

enum class ConductorMode
{
	None,
	LocalClientHost,
	Host,
	RemoteClient,
};

struct ConductorModeState
{
	virtual ~ConductorModeState() {}
};

struct ConductorState
{
	File::Path dataDirectory;
	File::Path userDirectory;

	Asset::AssetManager assetManager;
	Conductor::GameData gameData;

	ConductorMode mode = ConductorMode::None;
	ConductorModeState* modeState = nullptr;

	ConductorState(const char* _dataDirectory, const char* _userDirectory)
		: dataDirectory(_dataDirectory)
		, userDirectory(_userDirectory)
		, assetManager(dataDirectory)
		, gameData(dataDirectory, userDirectory, assetManager)
	{}
};

struct LocalClientHostState : ConductorModeState
{
	// TODO where does input come from
	static constexpr size_t k_clientRenderMessageCapacity = 256;
	Collection::LocklessQueue<Input::InputMessage> inputToClientMessages{ k_clientRenderMessageCapacity };

	Collection::LocklessQueue<Client::MessageToHost> clientToHostMessages{
		Host::HostNetworkWorld::k_inboundMessageCapacity };
	Collection::LocklessQueue<Host::MessageToClient> hostToClientMessages{
		Host::HostNetworkWorld::k_outboundMessageCapacityPerClient };

	Client::ClientWorld clientWorld;
	Host::HostWorld hostWorld;

	LocalClientHostState(Conductor::GameData& gameData)
		: clientWorld(gameData, inputToClientMessages, hostToClientMessages)
		, hostWorld(gameData, clientToHostMessages)
	{}

	virtual ~LocalClientHostState() {}
};

struct HostState : ConductorModeState
{
	Host::HostNetworkWorld hostNetworkWorld;
	Host::HostWorld hostWorld;

	HostState(Conductor::GameData& gameData, const char* port)
		: hostNetworkWorld(port)
		, hostWorld(gameData, hostNetworkWorld.GetClientToHostMessageQueue())
	{}

	virtual ~HostState()
	{
		// Block this thread until the host network thread stops.
		hostNetworkWorld.WaitForShutdown();
	}
};

struct RemoteClientState : ConductorModeState
{
	// TODO where does input come from
	static constexpr size_t k_clientRenderMessageCapacity = 256;
	Collection::LocklessQueue<Input::InputMessage> inputToClientMessages{ k_clientRenderMessageCapacity };

	Client::ClientNetworkWorld clientNetworkWorld;
	Client::ClientWorld clientWorld;

	RemoteClientState(Conductor::GameData& gameData, Client::ClientNetworkWorld&& _clientNetworkWorld)
		: clientNetworkWorld(std::move(_clientNetworkWorld))
		, clientWorld(gameData, inputToClientMessages, clientNetworkWorld.GetHostToClientMessageQueue())
	{}

	virtual ~RemoteClientState() {}
};

extern "C"
{

	// The asset/type system is separately initialized to reduce the overhead of having multiple clients/hosts
	// throughout the lifetime of the program.
	// - Start up the asset/type system
	// - Shut down the asset/type system
	// - Register an asset type?
	// - Register a component type?
	DLL_EXPORT ConductorState* Conductor_CreateCore(const char* dataDirectory, const char* userDirectory)
	{
		auto* conductorState = new ConductorState(dataDirectory, userDirectory);

		return conductorState;
	}

	DLL_EXPORT void Conductor_DestroyCore(ConductorState* conductorState)
	{
		if (conductorState->mode != ConductorMode::None)
		{
			// TODO error if the mode isn't "none"
		}
		delete conductorState;
	}

	// Start a combination client/host (with no network ability?)
	// Shut down a combination client/host
	// Check if the combination client/host is still running

	DLL_EXPORT void Conductor_CreateLocalClientHost(ConductorState* conductorState)
	{
		if (conductorState->mode != ConductorMode::None)
		{
			// TODO report error
			return;
		}

		auto* modeState = new LocalClientHostState(conductorState->gameData);

		constexpr Client::ClientID clientID = Host::HostNetworkWorld::k_localClientID;
		modeState->hostWorld.NotifyOfClientConnected(Mem::MakeUnique<Host::ConnectedClient>(clientID, modeState->hostToClientMessages));
		modeState->clientWorld.NotifyOfHostConnected(Mem::MakeUnique<Client::ConnectedHost>(clientID, modeState->clientToHostMessages));

		conductorState->mode = ConductorMode::LocalClientHost;
		conductorState->modeState = modeState;
	}

	DLL_EXPORT void Conductor_DestroyLocalClientHost(ConductorState* conductorState)
	{
		if (conductorState->mode != ConductorMode::LocalClientHost)
		{
			// TODO report error
			return;
		}

		conductorState->mode = ConductorMode::None;
		delete conductorState->modeState;
		conductorState->modeState = nullptr;
	}

	DLL_EXPORT bool Conductor_IsLocalClientHostRunning(ConductorState* conductorState)
	{
		if (conductorState->mode != ConductorMode::LocalClientHost)
		{
			return false;
		}
		auto* modeState = static_cast<LocalClientHostState*>(conductorState->modeState);
		return (modeState->hostWorld.GetNumConnectedClients() > 0);
	}

	// Start a host
	// Shut down a host
	// Send a console input to the host (allows control over most host aspects)

	DLL_EXPORT void Conductor_CreateHost(ConductorState* conductorState, const char* port)
	{
		// Initialize the network socket API.
		if (!Network::TryInitializeSocketAPI())
		{
			// TODO report error
			return;
		}

		// Setup the network world and verify it is running.
		auto* modeState = new HostState(conductorState->gameData, port);
		if (!modeState->hostNetworkWorld.IsRunning())
		{
			// TODO report error
			delete modeState;
			Network::ShutdownSocketAPI();
			return;
		}

		conductorState->mode = ConductorMode::Host;
		conductorState->modeState = modeState;
	}

	DLL_EXPORT void Conductor_DestroyHost(ConductorState* conductorState)
	{
		if (conductorState->mode != ConductorMode::Host)
		{
			// TODO report error
			return;
		}

		conductorState->mode = ConductorMode::None;
		delete conductorState->modeState;
		conductorState->modeState = nullptr;

		// Shutdown the socket API.
		Network::ShutdownSocketAPI();
	}

	DLL_EXPORT bool Conductor_IsHostRunning(ConductorState* conductorState)
	{
		if (conductorState->mode != ConductorMode::Host)
		{
			return false;
		}
		auto* modeState = static_cast<HostState*>(conductorState->modeState);
		return modeState->hostNetworkWorld.IsRunning();
	}

	DLL_EXPORT void Conductor_NotifyHostOfConsoleInput(ConductorState* conductorState, const char* input)
	{
		auto* modeState = static_cast<HostState*>(conductorState->modeState);
		if (conductorState->mode != ConductorMode::Host || !modeState->hostNetworkWorld.IsRunning())
		{
			// TODO report error
			return;
		}
		modeState->hostNetworkWorld.NotifyOfConsoleInput(std::string(input));
	}

	// Start up a remote client
	// Shut down a remote client
	// Check if remote client is still running

	DLL_EXPORT void Conductor_CreateRemoteClient(ConductorState* conductorState, const char* hostName, const char* hostPort)
	{
		// Initialize the network socket API.
		if (!Network::TryInitializeSocketAPI())
		{
			// TODO report error
			return;
		}

		// Establish a connection to the networked host.
		Client::ClientNetworkWorld clientNetworkWorld{ hostName, hostPort };
		if (!clientNetworkWorld.IsRunning())
		{
			// TODO report error
			Network::ShutdownSocketAPI();
			return;
		}

		// Create a client and notify it of the connection.
		auto* modeState = new RemoteClientState(conductorState->gameData, std::move(clientNetworkWorld)); 
		modeState->clientWorld.NotifyOfHostConnected(Mem::MakeUnique<Client::ConnectedHost>(
			modeState->clientNetworkWorld.GetClientID(), modeState->clientNetworkWorld.GetClientToHostMessageQueue()));

		conductorState->mode = ConductorMode::RemoteClient;
		conductorState->modeState = modeState;
	}

	DLL_EXPORT void Conductor_DestroyRemoteClient(ConductorState* conductorState)
	{
		if (conductorState->mode != ConductorMode::RemoteClient)
		{
			// TODO report error
			return;
		}

		conductorState->mode = ConductorMode::None;
		delete conductorState->modeState;
		conductorState->modeState = nullptr;

		// Shutdown the socket API.
		Network::ShutdownSocketAPI();
	}

	DLL_EXPORT bool Conductor_IsRemoteClientRunning(ConductorState* conductorState)
	{
		if (conductorState->mode != ConductorMode::RemoteClient)
		{
			return false;
		}
		// TODO how to tell
		return false;
	}
}
