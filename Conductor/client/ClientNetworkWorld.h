#pragma once

#include <client/MessageToHost.h>

#include <collection/LocklessQueue.h>
#include <host/MessageToClient.h>
#include <network/Socket.h>

#include <thread>

namespace Client
{
/**
 * ClientNetworkWorld abstracts the network layer away from ClientWorld,
 * transmitting and receiving messages across the network to and from a host.
 */
class ClientNetworkWorld
{
public:
	static constexpr size_t k_inboundMessageCapacity = 512;
	static constexpr size_t k_outboundMessageCapacity = 128;

	ClientNetworkWorld(const char* hostName, const char* hostPort);
	
	bool IsRunning() const;
	void WaitForShutdown();

	ClientID GetClientID() const { return m_clientID; }
	Collection::LocklessQueue<Client::MessageToHost>& GetClientToHostMessageQueue() { return m_clientToHostMessages; }
	Collection::LocklessQueue<Host::MessageToClient>& GetHostToClientMessageQueue() { return m_hostToClientMessages; }

private:
	void NetworkThreadFunction();

	Network::Socket m_socket;

	ClientID m_clientID{};
	Collection::LocklessQueue<Client::MessageToHost> m_clientToHostMessages{ k_outboundMessageCapacity };
	Collection::LocklessQueue<Host::MessageToClient> m_hostToClientMessages{ k_inboundMessageCapacity };
	
	std::thread m_networkThread{};
};
}
