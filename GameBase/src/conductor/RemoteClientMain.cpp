#include <conductor/RemoteClientMain.h>

#include <client/IRenderInstance.h>
#include <client/MessageToRenderInstance.h>
#include <collection/LocklessQueue.h>
#include <network/Socket.h>

Conductor::ApplicationErrorCode Conductor::RemoteClientMain(
	const Collection::ProgramParameters& params,
	const File::Path& dataDirectory,
	const char* const hostName,
	const char* const hostPort,
	Client::RenderInstanceFactory&& renderInstanceFactory,
	GameDataFactory&& gameDataFactory,
	Client::ClientWorld::ClientFactory&& clientFactory)
{
	// Create a render instance. Because a render instance creates a window,
	// it must be created and managed on the main thread.
	constexpr size_t k_clientRenderMessageCapacity = 256;
	Collection::LocklessQueue<Client::InputMessage> inputToClientMessages{ k_clientRenderMessageCapacity };
	Collection::LocklessQueue<Client::MessageToRenderInstance> clientToRenderInstanceMessages{
		k_clientRenderMessageCapacity };
	
	Mem::UniquePtr<Client::IRenderInstance> renderInstance =
		renderInstanceFactory(dataDirectory, clientToRenderInstanceMessages, inputToClientMessages);

	// Load data files.
	Mem::UniquePtr<IGameData> gameData = gameDataFactory(dataDirectory);

	// Initialize the network socket API.
	if (!Network::TryInitializeSocketAPI())
	{
		return ApplicationErrorCode::FailedToInitializeSocketAPI;
	}

	// Connect the client to a networked host.
	Network::Socket socket = Network::CreateConnectedSocket(hostName, hostPort);

	// TODO use the connection

	// Shutdown the socket API.
	Network::ShutdownSocketAPI();

	return ApplicationErrorCode::NoError;
}
