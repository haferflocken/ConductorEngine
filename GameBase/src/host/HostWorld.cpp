#include <host/HostWorld.h>

#include <host/ConnectedClient.h>
#include <host/IHost.h>

#include <collection/LocklessQueue.h>

Host::HostWorld::HostWorld(Collection::LocklessQueue<std::function<void()>>& networkInputQueue,
	HostFactory&& hostFactory)
	: m_networkInputQueue(networkInputQueue)
	, m_hostFactory(std::move(hostFactory))
{
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
	m_connectedClients.Add(std::move(connectedClient));
}

void Host::HostWorld::NotifyOfClientDisconnected(const uint16_t clientID)
{
	const size_t clientIndex = m_connectedClients.IndexOf([&](const Mem::UniquePtr<ConnectedClient>& connectedClient)
	{
		return connectedClient->GetClientID() == clientID;
	});
	if (clientIndex != m_connectedClients.sk_InvalidIndex)
	{
		m_connectedClients[clientIndex]->NotifyOfHostDisconnected();

		using std::swap;
		swap(m_connectedClients[clientIndex], m_connectedClients.Back());
		m_connectedClients.RemoveLast();
	}
}

void Host::HostWorld::HostThreadFunction()
{
	m_hostThreadStatus = HostThreadStatus::Running;
	m_host = m_hostFactory();

	while (m_hostThreadStatus == HostThreadStatus::Running)
	{
		// Process pending input from the network.
		std::function<void()> message;
		while (m_networkInputQueue.TryPop(message))
		{
			message();
		}

		m_host->Update();
		// TODO transmit game state to clients
		std::this_thread::yield();
	}

	m_host.Reset();
	m_hostThreadStatus = HostThreadStatus::Stopped;
}
