#include <client/ClientNetworkWorld.h>

Client::ClientNetworkWorld::ClientNetworkWorld(const char* hostName, const char* hostPort)
	: m_socket(Network::CreateConnectedSocket(hostName, hostPort))
{
	if (m_socket.IsValid())
	{
		// TODO(network) Get the client ID from the host.

		// Start the network thread after getting the client ID because
		// the network thread runs until the client ID is invalid.
		m_networkThread = std::thread(&ClientNetworkWorld::NetworkThreadFunction, this);
	}
}

bool Client::ClientNetworkWorld::IsRunning() const
{
	return m_networkThread.joinable();
}

void Client::ClientNetworkWorld::WaitForShutdown()
{
	m_networkThread.join();
}

void Client::ClientNetworkWorld::NetworkThreadFunction()
{
	// Run the thread so long as the client ID is valid.
	while (m_clientID.IsValid())
	{
		// TODO(network) Receive any pending data from the host.

		// Transmit client messages to the host.
		Client::MessageToHost messageToHost;
		while (m_clientToHostMessages.TryPop(messageToHost))
		{
			if (messageToHost.Is<Client::MessageToHost_Disconnect>())
			{
				m_clientID = ClientID();
			}

			// TODO(network) transmit the message over the network
		}

		std::this_thread::yield();
	}
}
