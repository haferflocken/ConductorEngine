#pragma once

#include <client/ClientID.h>

namespace Collection { template <typename T> class LocklessQueue; }
namespace Host { struct MessageToClient; }

namespace Client
{
struct MessageToHost;

// ConnectedHost defines an asynchronous interface which a client uses to send data to a host.
class ConnectedHost
{
	ClientID m_clientID;
	Collection::LocklessQueue<Client::MessageToHost>& m_clientToHostMessages;

public:
	ConnectedHost(ClientID clientID,
		Collection::LocklessQueue<Client::MessageToHost>& clientToHostMessages)
		: m_clientID(clientID)
		, m_clientToHostMessages(clientToHostMessages)
	{}

	ClientID GetClientID() const { return m_clientID; }

	// Connect to a host. In Client code, pass in null; in Host network code, pass in a non-null value.
	void Connect(Collection::LocklessQueue<Host::MessageToClient>* hostToClientMessages);

	void Disconnect();
};
}
