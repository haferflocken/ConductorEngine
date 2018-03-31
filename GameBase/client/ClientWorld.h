#pragma once

#include <mem/UniquePtr.h>

#include <functional>
#include <thread>

namespace Collection { template <typename T> class LocklessQueue; }
namespace Host { struct MessageToClient; }

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

	ClientWorld(Collection::LocklessQueue<Host::MessageToClient>& networkInputQueue,
		ClientFactory&& clientFactory);

	ClientWorld() = delete;
	ClientWorld(const ClientWorld&) = delete;
	ClientWorld(ClientWorld&&) = delete;

	~ClientWorld();

	void RequestShutdown();

	void NotifyOfHostConnected(Mem::UniquePtr<ConnectedHost>&& connectedHost);
	void NotifyOfHostDisconnected();

private:
	enum class ClientThreadStatus
	{
		Stopped,
		Running,
		ShutdownRequested,
	};

	void ClientThreadFunction();
	void ProcessMessageFromHost(Host::MessageToClient& message);

	Collection::LocklessQueue<Host::MessageToClient>& m_networkInputQueue;
	ClientFactory m_clientFactory;

	Mem::UniquePtr<ConnectedHost> m_connectedHost{};
	Mem::UniquePtr<IClient> m_client{};

	std::thread m_clientThread{};
	ClientThreadStatus m_clientThreadStatus{ ClientThreadStatus::Stopped };
};
}
