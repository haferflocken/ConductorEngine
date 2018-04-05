#pragma once

#include <client/ClientID.h>

namespace Collection { template <typename T> class LocklessQueue; }

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

	void Disconnect();
};
}
