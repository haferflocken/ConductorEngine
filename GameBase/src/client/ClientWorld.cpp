#include <client/ClientWorld.h>

#include <client/ConnectedHost.h>
#include <client/IClient.h>

#include <collection/LocklessQueue.h>
#include <dev/Dev.h>

Client::ClientWorld::ClientWorld(Collection::LocklessQueue<std::function<void()>>& networkInputQueue,
	ClientFactory&& clientFactory)
	: m_networkInputQueue(networkInputQueue)
	, m_clientFactory(std::move(clientFactory))
{}

Client::ClientWorld::~ClientWorld()
{
	RequestShutdown();
	m_clientThread.join();
}

void Client::ClientWorld::RequestShutdown()
{
	m_clientThreadStatus = ClientThreadStatus::ShutdownRequested;
}

void Client::ClientWorld::NotifyOfHostConnected(Mem::UniquePtr<ConnectedHost>&& connectedHost)
{
	Dev::FatalAssert(m_connectedHost == nullptr, "TODO disconnect");
	m_connectedHost = std::move(connectedHost);

	m_clientThread = std::thread(&ClientWorld::ClientThreadFunction, this);
}

void Client::ClientWorld::NotifyOfHostDisconnected()
{
	RequestShutdown();
}

void Client::ClientWorld::ClientThreadFunction()
{
	m_clientThreadStatus = ClientThreadStatus::Running;
	m_client = m_clientFactory(*m_connectedHost);

	while (m_clientThreadStatus == ClientThreadStatus::Running)
	{
		// Process pending input from the network.
		std::function<void()> message;
		while (m_networkInputQueue.TryPop(message))
		{
			message();
		}

		// TODO client loop
		m_client->Update();
		std::this_thread::yield();
	}

	m_client.Reset();
	m_connectedHost.Reset();
	m_clientThreadStatus = ClientThreadStatus::Stopped;
}
