#pragma once

#include <host/MessageToClient.h>
#include <mem/UniquePtr.h>

#include <chrono>
#include <functional>
#include <thread>

namespace Collection { template <typename T> class LocklessQueue; }
namespace Conductor { class IGameData; }
namespace Input { struct InputMessage; }

namespace Client
{
class ConnectedHost;
class IClient;
class IRenderInstance;

/**
 * ClientWorld connects to a host game simulation and allows a user to view it and interact with it.
 */
class ClientWorld final
{
public:
	using ClientFactory = std::function<Mem::UniquePtr<IClient>(const Conductor::IGameData&, ConnectedHost&)>;

	ClientWorld(const Conductor::IGameData& gameData,
		IRenderInstance& renderInstance,
		Collection::LocklessQueue<Input::InputMessage>& inputMessages,
		Collection::LocklessQueue<Host::MessageToClient>& networkInputQueue,
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
	void ProcessInputMessage(Input::InputMessage& message);

	const Conductor::IGameData& m_gameData;
	IRenderInstance& m_renderInstance;
	Collection::LocklessQueue<Input::InputMessage>& m_inputMessages;
	Collection::LocklessQueue<Host::MessageToClient>& m_networkInputQueue;
	ClientFactory m_clientFactory;

	Mem::UniquePtr<ConnectedHost> m_connectedHost{};
	Mem::UniquePtr<IClient> m_client{};

	std::chrono::steady_clock::time_point m_lastUpdatePoint;

	std::thread m_clientThread{};
	ClientThreadStatus m_clientThreadStatus{ ClientThreadStatus::Stopped };
};
}
