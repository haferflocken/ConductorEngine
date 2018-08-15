#pragma once

#include <client/ClientID.h>
#include <host/MessageToClient.h>

namespace Collection { template <typename T> class LocklessQueue; }

namespace Host
{
// ConnectedHost defines an asynchronous interface which a host uses to send data to a client.
class ConnectedClient
{
	Client::ClientID m_clientID;
	Collection::LocklessQueue<Host::MessageToClient>& m_hostToClientMessages;

public:
	ConnectedClient(Client::ClientID clientID,
		Collection::LocklessQueue<Host::MessageToClient>& hostToClientMessages)
			: m_clientID(clientID)
			, m_hostToClientMessages(hostToClientMessages)
		{}

	Client::ClientID GetClientID() const { return m_clientID; }

	void NotifyOfHostDisconnected();
};
}
