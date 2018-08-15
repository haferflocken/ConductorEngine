#include <host/HostNetworkWorld.h>

#include <client/ConnectedHost.h>

Host::HostNetworkWorld::HostNetworkWorld(const char* listenerPort)
	: m_listenerSocket(Network::CreateAndBindListenerSocket(listenerPort))
{
	if (m_listenerSocket.TryListen())
	{
		// Add the default local client for the server administrator.
		m_networkConnectedClients[k_localClientID] = Mem::MakeUnique<NetworkConnectedClient>();
		{
			Client::ConnectedHost localHost{ k_localClientID, m_clientToHostMessageQueue };
			localHost.Connect(&m_networkConnectedClients[k_localClientID]->m_hostToClientMessageQueue);
		}

		// Start the network thread after adding the default local client because the network thread terminates
		// when the default local client is disconnected.
		m_networkThread = std::thread(&HostNetworkWorld::NetworkThreadFunction, this);
	}
}

bool Host::HostNetworkWorld::IsRunning() const
{
	return m_networkThread.joinable();
}

void Host::HostNetworkWorld::WaitForShutdown()
{
	m_networkThread.join();
}

void Host::HostNetworkWorld::NotifyOfConsoleInput(std::string&& input)
{
	m_consoleMessageQueue.TryPush(std::move(input));
}

namespace Internal_HostNetworkWorld
{
void ProcessConsoleMessage(Client::ConnectedHost& localHost, const std::string& message)
{
	const char* const cMessage = message.c_str();
	if (strcmp(cMessage, "exit") == 0 || strcmp(cMessage, "quit") == 0)
	{
		localHost.Disconnect();
	}
	// TODO(network) kick clients
	// TODO(network) print client list
}
}

void Host::HostNetworkWorld::NetworkThreadFunction()
{
	// Run the thread so long as the default local client is not disconnected.
	while (m_networkConnectedClients.Find(k_localClientID) != m_networkConnectedClients.end())
	{
		// Process console messages.
		{
			// Create a Client::ConnectedHost for the local client to make processing simpler.
			// This is valid because Client::ConnectedHost just wraps this thread's client to host message queue.
			Client::ConnectedHost localHost{ k_localClientID, m_clientToHostMessageQueue };

			std::string message;
			while (m_consoleMessageQueue.TryPop(message))
			{
				Internal_HostNetworkWorld::ProcessConsoleMessage(localHost, message);
			}
		}

		// Accept client connection requests.
		constexpr size_t k_maxNewClients = 8;
		Network::Socket newClientSockets[k_maxNewClients];
		const size_t numNewClients = m_listenerSocket.AcceptPendingConnections(newClientSockets, k_maxNewClients);
		
		for (size_t i = 0; i < numNewClients; ++i)
		{
			const Client::ClientID clientID{
				static_cast<uint16_t>(k_localClientID.GetN() + m_networkConnectedClients.Size() + 1) };

			Dev::Log("New client connection accepted. ID: %u", clientID.GetN());

			Mem::UniquePtr<NetworkConnectedClient>& networkConnectedClient = m_networkConnectedClients[clientID];
			networkConnectedClient = Mem::MakeUnique<NetworkConnectedClient>();
			networkConnectedClient->m_clientSocket = std::move(newClientSockets[i]);
		}

		// Process the network connected clients.
		Collection::Vector<Client::ClientID> disconnectedClientIDs;
		for (auto& entry : m_networkConnectedClients)
		{
			const Client::ClientID clientID = entry.first;
			Mem::UniquePtr<NetworkConnectedClient>& networkConnectedClient = entry.second;

			// TODO(network) Receive any pending data from the client.

			// TODO(network) Process client to host messages, including disconnection messages.
			
			// TODO(network) Check if the client disconnected without notice.

			// Transmit host messages to each client.
			Host::MessageToClient messageToClient;
			while (networkConnectedClient->m_hostToClientMessageQueue.TryPop(messageToClient))
			{
				if (messageToClient.Is<Host::NotifyOfHostDisconnected_MessageToClient>())
				{
					disconnectedClientIDs.Add(clientID);
				}

				// TODO(network) transmit the message over the network
			}
		}

		// Remove any clients that are no longer connected.
		for (const auto& clientID : disconnectedClientIDs)
		{
			m_networkConnectedClients.TryRemove(clientID);
		}

		std::this_thread::yield();
	}
}
