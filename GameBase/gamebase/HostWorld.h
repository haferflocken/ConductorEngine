#pragma once

#include <collection/Vector.h>
#include <mem/UniquePtr.h>

namespace GameBase
{
class ConnectedClient;
class IHost;

/**
 * HostWorld runs a headless game simulation which clients can connect to and interact with.
 */
class HostWorld final
{
public:
	using HostFactory = std::function<Mem::UniquePtr<IHost>()>;

	HostWorld(HostFactory&& hostFactory);

	HostWorld() = delete;
	HostWorld(const HostWorld&) = delete;
	HostWorld(HostWorld&&) = delete;

	~HostWorld();

private:
	HostFactory m_hostFactory;
	Mem::UniquePtr<IHost> m_host;

	Collection::Vector<Mem::UniquePtr<ConnectedClient>> m_connectedClients;
};
}
