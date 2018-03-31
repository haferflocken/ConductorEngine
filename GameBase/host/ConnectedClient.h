#pragma once

#include <stdint.h>

namespace Host
{
// ConnectedHost defines an asynchronous interface which a host uses to send data to a client.
class ConnectedClient
{
protected:
	uint16_t m_clientID{ 0 };

public:
	uint16_t GetClientID() const { return m_clientID; }

	virtual void NotifyOfHostDisconnected() = 0;
};
}
