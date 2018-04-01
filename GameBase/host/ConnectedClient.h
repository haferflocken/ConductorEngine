#pragma once

#include <stdint.h>

namespace Collection { template <typename T> class LocklessQueue; }

namespace Host
{
struct MessageToClient;

// ConnectedHost defines an asynchronous interface which a host uses to send data to a client.
class ConnectedClient
{
	uint16_t m_clientID;
	Collection::LocklessQueue<Host::MessageToClient>& m_hostToClientMessages;

public:
	ConnectedClient(uint16_t clientID,
		Collection::LocklessQueue<Host::MessageToClient>& hostToClientMessages)
			: m_clientID(clientID)
			, m_hostToClientMessages(hostToClientMessages)
		{}

	uint16_t GetClientID() const { return m_clientID; }

	void NotifyOfHostDisconnected();
};
}
