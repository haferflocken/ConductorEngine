#include <host/HostWorld.h>

#include <client/MessageToHost.h>
#include <collection/LocklessQueue.h>
#include <dev/Dev.h>
#include <host/ConnectedClient.h>
#include <host/IHost.h>

Host::HostWorld::HostWorld(const Conductor::IGameData& gameData,
	Collection::LocklessQueue<Client::MessageToHost>& networkInputQueue,
	HostFactory&& hostFactory)
	: m_gameData(gameData)
	, m_networkInputQueue(networkInputQueue)
	, m_hostFactory(std::move(hostFactory))
	, m_lastUpdatePoint()
{
	// Acquire the host lock before the host thread is created because the host thread will release it once
	// the host is created. The memory order of this initial operation doesn't matter.
	m_hostLock.test_and_set();
	m_hostThread = std::thread(&HostWorld::HostThreadFunction, this);
}

Host::HostWorld::~HostWorld()
{
	RequestShutdown();
	m_hostThread.join();
}

void Host::HostWorld::RequestShutdown()
{
	m_hostThreadStatus = HostThreadStatus::ShutdownRequested;
}

void Host::HostWorld::NotifyOfClientConnected(Mem::UniquePtr<ConnectedClient>&& connectedClient)
{
	// This function may be called by different threads.
	while (m_hostLock.test_and_set(std::memory_order_acquire));

	const Client::ClientID clientID = connectedClient->GetClientID();
	const Input::InputStateManager& clientInputManager = connectedClient->GetClientInputStateManager();

	// Moving connectedClient into m_connectedClients doesn't invalidate the above reference because
	// sconnectedClient is a Mem::UniquePtr.
	m_connectedClients.Add(std::move(connectedClient));

	m_host->NotifyOfClientConnected(clientID, clientInputManager);

	m_hostLock.clear(std::memory_order_release);
}

void Host::HostWorld::NotifyOfClientDisconnected(const Client::ClientID clientID)
{
	// This function may be called by different threads.
	while (m_hostLock.test_and_set(std::memory_order_acquire));

	const size_t clientIndex = m_connectedClients.IndexOf([&](const Mem::UniquePtr<ConnectedClient>& connectedClient)
	{
		return connectedClient->GetClientID() == clientID;
	});
	if (clientIndex != m_connectedClients.sk_InvalidIndex)
	{
		m_host->NotifyOfClientDisconnected(clientID);

		m_connectedClients[clientIndex]->NotifyOfHostDisconnected();

		using std::swap;
		swap(m_connectedClients[clientIndex], m_connectedClients.Back());
		m_connectedClients.RemoveLast();
	}

	m_hostLock.clear(std::memory_order_release);
}

void Host::HostWorld::HostThreadFunction()
{
	m_hostThreadStatus = HostThreadStatus::Running;
	m_host = m_hostFactory(m_gameData);
	m_lastUpdatePoint = std::chrono::steady_clock::now();

	// Unlock the host. It was locked in the HostWorld constructor.
	m_hostLock.clear(std::memory_order_release);

	while (m_hostThreadStatus == HostThreadStatus::Running)
	{
		// Process pending input from the network.
		Client::MessageToHost message;
		while (m_networkInputQueue.TryPop(message))
		{
			ProcessMessageFromClient(message);
		}

		// Lock the host while the it and connected clients shouldn't be modified.
		while (m_hostLock.test_and_set(std::memory_order_acquire));

		// Update the game simulation.
		const auto nowPoint = std::chrono::steady_clock::now();
		const auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(nowPoint - m_lastUpdatePoint);
		m_host->Update(Unit::Time::Millisecond(deltaMs.count()));
		m_lastUpdatePoint = nowPoint;

		// Transmit ECS state to clients. So long as the host implements the networked part of their game simulation
		// using entities and components, this is all that needs to be sent.
		const Collection::Vector<uint8_t> ecsUpdateTransmission = m_host->SerializeECSUpdateTransmission();
		for (auto& connectedClient : m_connectedClients)
		{
			connectedClient->TransmitECSUpdate(ecsUpdateTransmission);
		}

		// Unlock the host when the it and connected clients may be modified again.
		m_hostLock.clear(std::memory_order_release);

		std::this_thread::yield();
	}

	// Lock the host permanently before destroying it. No other threads should attempt to modify it at this point.
	while (m_hostLock.test_and_set(std::memory_order_acquire));
	m_host.Reset();
	m_hostThreadStatus = HostThreadStatus::Stopped;
}

void Host::HostWorld::ProcessMessageFromClient(Client::MessageToHost& message)
{
	switch (message.m_type)
	{
	case Client::MessageToHostType::Connect:
	{
		AMP_FATAL_ASSERT(message.m_connectPayload.m_hostToClientMessages != nullptr,
			"HostWorld expects connect messages to have their message queues resolved in HostNetworkWorld.");
		NotifyOfClientConnected(
			Mem::MakeUnique<Host::ConnectedClient>(message.m_clientID, *message.m_connectPayload.m_hostToClientMessages));
		break;
	}
	case Client::MessageToHostType::Disconnect:
	{
		NotifyOfClientDisconnected(message.m_clientID);
		break;
	}
	}
}
