#pragma once

#include <client/ClientID.h>
#include <collection/Vector.h>
#include <host/MessageToClient.h>
#include <mem/UniquePtr.h>

#include <atomic>
#include <chrono>
#include <thread>

namespace Client { struct MessageToHost; }
namespace Collection { template <typename T> class LocklessQueue; }
namespace Conductor { class GameData; }

namespace Host
{
class ConnectedClient;
class HostInstance;

/**
 * HostWorld runs a headless game simulation which clients can connect to and interact with.
 */
class HostWorld final
{
public:
	HostWorld(const Conductor::GameData& gameData,
		Collection::LocklessQueue<Client::MessageToHost>& networkInputQueue);

	HostWorld() = delete;
	HostWorld(const HostWorld&) = delete;
	HostWorld(HostWorld&&) = delete;

	HostWorld& operator=(const HostWorld&) = delete;
	HostWorld& operator=(HostWorld&&) = delete;

	~HostWorld();

	uint32_t GetNumConnectedClients() const { return m_connectedClients.Size(); }

	void RequestShutdown();

	void NotifyOfClientConnected(Mem::UniquePtr<ConnectedClient>&& connectedClient);

private:
	enum class HostThreadStatus
	{
		Stopped,
		Running,
		ShutdownRequested,
	};

	void HostThreadFunction();
	void ProcessMessageFromClient(Client::MessageToHost& message);

	void NotifyOfClientDisconnected(const Client::ClientID clientID);
	void NotifyOfFrameAcknowledgement(const Client::ClientID clientID, const uint64_t frameIndex);
	void NotifyOfInputStateTransmission(const Client::ClientID clientID,
		const Collection::Vector<uint8_t>& transmissionBytes);

	const Conductor::GameData& m_gameData;
	Collection::LocklessQueue<Client::MessageToHost>& m_networkInputQueue;
	Mem::UniquePtr<HostInstance> m_host{};
	std::atomic_flag m_hostLock{};

	std::chrono::steady_clock::time_point m_lastUpdatePoint;
	std::thread m_hostThread{};
	HostThreadStatus m_hostThreadStatus{ HostThreadStatus::Stopped };

	Collection::Vector<Mem::UniquePtr<ConnectedClient>> m_connectedClients{};
};
}
