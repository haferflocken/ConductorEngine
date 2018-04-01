#pragma once

#include <stdint.h>

namespace Collection { template <typename T> class LocklessQueue; }

namespace Client
{
struct MessageToHost;

// ConnectedHost defines an asynchronous interface which a client uses to send data to a host.
class ConnectedHost
{
	uint16_t m_clientID;
	Collection::LocklessQueue<Client::MessageToHost>& m_clientToHostMessages;

public:
	ConnectedHost(uint16_t clientID,
		Collection::LocklessQueue<Client::MessageToHost>& clientToHostMessages)
		: m_clientID(clientID)
		, m_clientToHostMessages(clientToHostMessages)
	{}

	uint16_t GetClientID() const { return m_clientID; }

	void Disconnect();
};
}
