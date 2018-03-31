#pragma once

#include <stdint.h>

namespace Client
{
// ConnectedHost defines an asynchronous interface which a client uses to send data to a host.
class ConnectedHost
{
protected:
	uint16_t m_clientID{ 0 };

public:
	uint16_t GetClientID() const { return m_clientID; }

	virtual void Disconnect() = 0;
};
}
