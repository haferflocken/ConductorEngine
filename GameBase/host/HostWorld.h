#pragma once

#include <collection/Vector.h>
#include <mem/UniquePtr.h>

#include <thread>

namespace Host
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

	HostWorld& operator=(const HostWorld&) = delete;
	HostWorld& operator=(HostWorld&&) = delete;

	~HostWorld();

	void RequestShutdown();

private:
	enum class HostThreadStatus
	{
		Stopped,
		Running,
		ShutdownRequested,
	};

	void HostThreadFunction();

	HostFactory m_hostFactory;
	Mem::UniquePtr<IHost> m_host{};

	std::thread m_hostThread{};
	HostThreadStatus m_hostThreadStatus{ HostThreadStatus::Stopped };

	Collection::Vector<Mem::UniquePtr<ConnectedClient>> m_connectedClients{};
};
}
