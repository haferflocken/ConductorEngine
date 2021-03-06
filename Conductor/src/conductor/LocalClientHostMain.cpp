#include <conductor/LocalClientHostMain.h>

#include <asset/AssetManager.h>
#include <client/ConnectedHost.h>
#include <client/MessageToRenderInstance.h>
#include <collection/LocklessQueue.h>
#include <host/ConnectedClient.h>
#include <host/HostNetworkWorld.h>
#include <host/HostWorld.h>
#include <input/InputMessage.h>

Conductor::ApplicationErrorCode Conductor::LocalClientHostMain(
	const Collection::ProgramParameters& params,
	const File::Path& dataDirectory,
	const File::Path& userDirectory,
	Asset::AssetManager& assetManager,
	Client::RenderInstanceFactory&& renderInstanceFactory,
	GameDataFactory&& gameDataFactory,
	Client::ClientWorld::ClientFactory&& clientFactory,
	Host::HostWorld::HostFactory&& hostFactory)
{
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

	// Create the message queues that will allow the client and host to communicate.
	Collection::LocklessQueue<Client::MessageToHost> clientToHostMessages{
		Host::HostNetworkWorld::k_inboundMessageCapacity };
	Collection::LocklessQueue<Host::MessageToClient> hostToClientMessages{
		Host::HostNetworkWorld::k_outboundMessageCapacityPerClient };

	// Create the client and connect it to a new host.
	Client::ClientWorld clientWorld{ *gameData, *renderInstance, inputToClientMessages,
		hostToClientMessages, std::move(clientFactory) };
	Host::HostWorld hostWorld{ *gameData, clientToHostMessages, std::move(hostFactory) };
	
	constexpr Client::ClientID clientID = Host::HostNetworkWorld::k_localClientID;
	hostWorld.NotifyOfClientConnected(Mem::MakeUnique<Host::ConnectedClient>(clientID, hostToClientMessages));
	clientWorld.NotifyOfHostConnected(Mem::MakeUnique<Client::ConnectedHost>(clientID, clientToHostMessages));

	// Run the window while the client and host run in other threads. Stop when the host stops.
	while (renderInstance->Update() == Client::IRenderInstance::Status::Running
		|| hostWorld.GetNumConnectedClients() > 0)
	{
		std::this_thread::yield();
	}

	return ApplicationErrorCode::NoError;
}
