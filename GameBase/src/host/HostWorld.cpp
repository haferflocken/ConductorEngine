#include <host/HostWorld.h>

#include <client/MessageToHost.h>
#include <collection/LocklessQueue.h>
#include <dev/Dev.h>

#include <host/ConnectedClient.h>
#include <host/IHost.h>

Host::HostWorld::HostWorld(Collection::LocklessQueue<Client::MessageToHost>& networkInputQueue,
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

void Host::HostWorld::NotifyOfClientDisconnected(const Client::ClientID clientID)
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
		Client::MessageToHost message;
		while (m_networkInputQueue.TryPop(message))
		{
			ProcessMessageFromClient(message);
		}

		// Update the game simulation.
		m_host->Update();

		// TODO transmit game state to clients
		std::this_thread::yield();
	}

	m_host.Reset();
	m_hostThreadStatus = HostThreadStatus::Stopped;
}

void Host::HostWorld::ProcessMessageFromClient(Client::MessageToHost& message)
{
	switch (message.m_type)
	{
	case Client::MessageToHostType::Connect:
	{
		Dev::FatalAssert(message.m_connectPayload.m_hostToClientMessages != nullptr,
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
