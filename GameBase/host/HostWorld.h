#pragma once

#include <collection/Vector.h>
#include <mem/UniquePtr.h>

#include <thread>

namespace Client { struct MessageToHost; }

namespace Collection { template <typename T> class LocklessQueue; }

namespace Host
{
class ConnectedClient;
class IHost;
struct MessageToClient;

/**
 * HostWorld runs a headless game simulation which clients can connect to and interact with.
 */
class HostWorld final
{
public:
	using HostFactory = std::function<Mem::UniquePtr<IHost>()>;

	HostWorld(Collection::LocklessQueue<Client::MessageToHost>& networkInputQueue,
		HostFactory&& hostFactory);

	HostWorld() = delete;
	HostWorld(const HostWorld&) = delete;
	HostWorld(HostWorld&&) = delete;

	HostWorld& operator=(const HostWorld&) = delete;
	HostWorld& operator=(HostWorld&&) = delete;

	~HostWorld();

	void RequestShutdown();

	void NotifyOfClientConnected(Mem::UniquePtr<ConnectedClient>&& connectedClient);
	void NotifyOfClientDisconnected(const uint16_t clientID);
	uint32_t GetNumConnectedClients() const { return m_connectedClients.Size(); }

private:
	enum class HostThreadStatus
	{
		Stopped,
		Running,
		ShutdownRequested,
	};

	void HostThreadFunction();
	void ProcessMessageFromClient(Client::MessageToHost& message);
	
	Collection::LocklessQueue<Client::MessageToHost>& m_networkInputQueue;
	HostFactory m_hostFactory;
	Mem::UniquePtr<IHost> m_host{};

	std::thread m_hostThread{};
	HostThreadStatus m_hostThreadStatus{ HostThreadStatus::Stopped };

	Collection::Vector<Mem::UniquePtr<ConnectedClient>> m_connectedClients{};
};
}
