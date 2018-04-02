#include <client/ClientWorld.h>

#include <client/ConnectedHost.h>
#include <client/IClient.h>
#include <client/InputMessage.h>

#include <collection/LocklessQueue.h>
#include <dev/Dev.h>
#include <host/MessageToClient.h>

Client::ClientWorld::ClientWorld(Collection::LocklessQueue<Client::InputMessage>& inputMessages,
	Collection::LocklessQueue<Host::MessageToClient>& networkInputQueue, ClientFactory&& clientFactory)
	: m_inputMessages(inputMessages)
	, m_networkInputQueue(networkInputQueue)
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
		{
			Host::MessageToClient message;
			while (m_networkInputQueue.TryPop(message))
			{
				ProcessMessageFromHost(message);
			}
		}

		// Process pending player input.
		{
			Client::InputMessage message;
			while (m_inputMessages.TryPop(message))
			{
				ProcessInputMessage(message);
			}
		}

		// TODO queue player input for transmission on the network
		// TODO predict simulation state based on player input
		// TODO interpolate between the last received server state and the predicted simulation state

		m_client->Update();
		std::this_thread::yield();
	}

	m_client.Reset();
	m_connectedHost.Reset();
	m_clientThreadStatus = ClientThreadStatus::Stopped;
}

void Client::ClientWorld::ProcessMessageFromHost(Host::MessageToClient& message)
{
	switch (message.m_type)
	{
	case Host::MessageToClientType::NotifyOfHostDisconnected:
	{
		NotifyOfHostDisconnected();
		break;
	}
	}
}

void Client::ClientWorld::ProcessInputMessage(Client::InputMessage& message)
{
	switch (message.m_type)
	{
	case Client::InputMessageType::WindowClosed:
	{
		m_client->NotifyOfWindowClosed();
		m_connectedHost->Disconnect();
		break;
	}
	case Client::InputMessageType::KeyUp:
	{
		m_client->NotifyOfKeyUp(message.m_key);
		break;
	}
	case Client::InputMessageType::KeyDown:
	{
		m_client->NotifyOfKeyDown(message.m_key);
		break;
	}
	default:
	{
		m_client->NotifyOfInputMessage(message);
		break;
	}
	}
}
