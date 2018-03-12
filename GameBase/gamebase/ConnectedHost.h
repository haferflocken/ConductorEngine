#pragma once

namespace GameBase
{
// ConnectedHost defines an asynchronous interface which a client uses to connect with a host.
class ConnectedHost
{
public:
	virtual bool IsHostUpdateAvailable() const = 0;
	// virtual ??? GetHostUpdate() = 0;
};
}
