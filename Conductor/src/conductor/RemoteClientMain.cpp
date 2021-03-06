#include <conductor/RemoteClientMain.h>

#include <asset/AssetManager.h>
#include <client/ConnectedHost.h>
#include <client/IRenderInstance.h>
#include <client/MessageToRenderInstance.h>
#include <client/ClientNetworkWorld.h>
#include <collection/LocklessQueue.h>
#include <input/InputMessage.h>
#include <network/Socket.h>

Conductor::ApplicationErrorCode Conductor::RemoteClientMain(
	const Collection::ProgramParameters& params,
	const File::Path& dataDirectory,
	const File::Path& userDirectory,
	Asset::AssetManager& assetManager,
	const char* const hostName,
	const char* const hostPort,
	Client::RenderInstanceFactory&& renderInstanceFactory,
	GameDataFactory&& gameDataFactory,
	Client::ClientWorld::ClientFactory&& clientFactory)
{
	// Initialize the network socket API.
	if (!Network::TryInitializeSocketAPI())
	{
		return ApplicationErrorCode::FailedToInitializeSocketAPI;
	}

	// Initialize asset types, register component types, and load game data.
	Mem::UniquePtr<IGameData> gameData = gameDataFactory(assetManager, dataDirectory, userDirectory);

	// Create a render instance. Because a render instance creates a window,
	// it must be created and managed on the main thread.
	constexpr size_t k_clientRenderMessageCapacity = 256;
	Collection::LocklessQueue<Input::InputMessage> inputToClientMessages{ k_clientRenderMessageCapacity };
	Collection::LocklessQueue<Client::MessageToRenderInstance> clientToRenderInstanceMessages{
		k_clientRenderMessageCapacity };
	
	Mem::UniquePtr<Client::IRenderInstance> renderInstance =
		renderInstanceFactory(assetManager, dataDirectory, clientToRenderInstanceMessages, inputToClientMessages);

	// Establish a connection to the networked host.
	Client::ClientNetworkWorld clientNetworkWorld{ hostName, hostPort };
	if (!clientNetworkWorld.IsRunning())
	{
		Network::ShutdownSocketAPI();
		return ApplicationErrorCode::FailedToInitializeNetworkThread;
	}

	// Create a client and notify it of the connection.
	Client::ClientWorld clientWorld{ *gameData, *renderInstance, inputToClientMessages,
		clientNetworkWorld.GetHostToClientMessageQueue(), std::move(clientFactory) };
	clientWorld.NotifyOfHostConnected(Mem::MakeUnique<Client::ConnectedHost>(
		clientNetworkWorld.GetClientID(), clientNetworkWorld.GetClientToHostMessageQueue()));

	// Run the window while the client runs in another thread.
	while (renderInstance->Update() == Client::IRenderInstance::Status::Running)
	{
		std::this_thread::yield();
	}

	// Shutdown the socket API.
	Network::ShutdownSocketAPI();

	return ApplicationErrorCode::NoError;
}
