#pragma once

#include <host/MessageToClient.h>
#include <mem/UniquePtr.h>

#include <chrono>
#include <functional>
#include <thread>

namespace Collection { template <typename T> class LocklessQueue; }
namespace Conductor { class GameData; }
namespace Input { struct InputMessage; }
namespace Math { class Frustum; }

namespace Client
{
class ConnectedHost;
class ClientInstance;

/**
 * ClientWorld connects to a host game simulation and allows a user to view it and interact with it.
 */
class ClientWorld final
{
public:
	ClientWorld(const Conductor::GameData& gameData,
		Collection::LocklessQueue<Input::InputMessage>& inputMessages,
		Collection::LocklessQueue<Host::MessageToClient>& networkInputQueue);

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
	void ProcessInputMessage(Input::InputMessage& message);

	const Conductor::GameData& m_gameData;
	Collection::LocklessQueue<Input::InputMessage>& m_inputMessages;
	Collection::LocklessQueue<Host::MessageToClient>& m_networkInputQueue;

	Mem::UniquePtr<ConnectedHost> m_connectedHost{};
	Mem::UniquePtr<ClientInstance> m_client{};

	std::chrono::steady_clock::time_point m_lastUpdatePoint;

	std::thread m_clientThread{};
	ClientThreadStatus m_clientThreadStatus{ ClientThreadStatus::Stopped };
};
}
