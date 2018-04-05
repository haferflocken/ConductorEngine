#include <host/HostNetworkWorld.h>

#include <client/ConnectedHost.h>

Host::HostNetworkWorld::HostNetworkWorld()
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

		// TODO thread loop:
		//      Accept or reject client connection requests
		//      Process client messages, including disconnections
		//      Transmit host messages to clients

		Collection::Vector<Client::ClientID> disconnectedClientIDs;

		// Transmit host messages to each client.
		for (auto& entry : m_networkConnectedClients)
		{
			const Client::ClientID clientID = entry.first;
			Mem::UniquePtr<NetworkConnectedClient>& networkConnectedClient = entry.second;

			Host::MessageToClient message;
			while (networkConnectedClient->m_hostToClientMessageQueue.TryPop(message))
			{
				if (message.m_type == Host::MessageToClientType::NotifyOfHostDisconnected)
				{
					disconnectedClientIDs.Add(clientID);
				}

				// TODO transmit the message over the network
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
