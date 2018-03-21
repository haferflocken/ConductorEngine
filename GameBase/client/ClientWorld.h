#pragma once

#include <mem/UniquePtr.h>

#include <functional>

namespace Client
{
class ConnectedHost;
class IClient;

/**
 * ClientWorld connects to a host game simulation and allows a user to view it and interact with it.
 */
class ClientWorld final
{
public:
	using ClientFactory = std::function<Mem::UniquePtr<IClient>(ConnectedHost&)>;

	ClientWorld(ClientFactory&& clientFactory);

	ClientWorld() = delete;
	ClientWorld(const ClientWorld&) = delete;
	ClientWorld(ClientWorld&&) = delete;

	~ClientWorld();

private:
	ClientFactory m_clientFactory;
	Mem::UniquePtr<ConnectedHost> m_connectedHost{};
	Mem::UniquePtr<IClient> m_client{};
};
}
