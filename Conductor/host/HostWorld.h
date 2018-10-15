#pragma once

#include <client/ClientID.h>
#include <collection/Vector.h>
#include <host/MessageToClient.h>
#include <mem/UniquePtr.h>

#include <chrono>
#include <thread>

namespace Client { struct MessageToHost; }
namespace Collection { template <typename T> class LocklessQueue; }
namespace Conductor { class IGameData; }

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
	using HostFactory = std::function<Mem::UniquePtr<IHost>(const Conductor::IGameData&)>;

	HostWorld(const Conductor::IGameData& gameData,
		Collection::LocklessQueue<Client::MessageToHost>& networkInputQueue,
		HostFactory&& hostFactory);

	HostWorld() = delete;
	HostWorld(const HostWorld&) = delete;
	HostWorld(HostWorld&&) = delete;

	HostWorld& operator=(const HostWorld&) = delete;
	HostWorld& operator=(HostWorld&&) = delete;

	~HostWorld();

	void RequestShutdown();

	void NotifyOfClientConnected(Mem::UniquePtr<ConnectedClient>&& connectedClient);
	void NotifyOfClientDisconnected(const Client::ClientID clientID);
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
	
	const Conductor::IGameData& m_gameData;
	Collection::LocklessQueue<Client::MessageToHost>& m_networkInputQueue;
	HostFactory m_hostFactory;
	Mem::UniquePtr<IHost> m_host{};

	std::chrono::steady_clock::time_point m_lastUpdatePoint;

	std::thread m_hostThread{};
	HostThreadStatus m_hostThreadStatus{ HostThreadStatus::Stopped };

	Collection::Vector<Mem::UniquePtr<ConnectedClient>> m_connectedClients{};
};
}
