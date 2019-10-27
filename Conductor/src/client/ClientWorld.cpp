#include <client/ClientWorld.h>

#include <conductor/GameData.h>
#include <client/ConnectedHost.h>
#include <client/ClientInstance.h>
#include <client/IRenderInstance.h>
#include <input/InputMessage.h>

#include <collection/LocklessQueue.h>
#include <dev/Dev.h>
#include <host/MessageToClient.h>

Client::ClientWorld::ClientWorld(const Conductor::GameData& gameData,
	Collection::LocklessQueue<Input::InputMessage>& inputMessages,
	Collection::LocklessQueue<Host::MessageToClient>& networkInputQueue)
	: m_gameData(gameData)
	, m_inputMessages(inputMessages)
	, m_networkInputQueue(networkInputQueue)
	, m_lastUpdatePoint()
{}

Client::ClientWorld::~ClientWorld()
{
	if (m_clientThreadStatus == ClientThreadStatus::Running)
	{
		RequestShutdown();
	}
	m_clientThread.join();
}

void Client::ClientWorld::RequestShutdown()
{
	m_clientThreadStatus = ClientThreadStatus::ShutdownRequested;
}

void Client::ClientWorld::NotifyOfHostConnected(Mem::UniquePtr<ConnectedHost>&& connectedHost)
{
	AMP_FATAL_ASSERT(m_connectedHost == nullptr, "TODO disconnect");
	AMP_FATAL_ASSERT(connectedHost->GetClientID().IsValid(), "Can't be connected to a host with an invalid ClientID!");
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
	m_client = Mem::MakeUnique<ClientInstance>(m_gameData, *m_connectedHost);
	m_lastUpdatePoint = std::chrono::steady_clock::now();

	// Acknowledge the connection to the host.
	m_connectedHost->Connect(nullptr);

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

		// Process pending player input and then queue it for transmission on the network.
		{
			Input::InputMessage message;
			while (m_inputMessages.TryPop(message))
			{
				ProcessInputMessage(message);
			}

			m_connectedHost->TransmitInputStates(m_client->SerializeInputStateTransmission());
		}

		// TODO predict simulation state based on player input
		// TODO interpolate between the last received server state and the predicted simulation state

		const auto nowPoint = std::chrono::steady_clock::now();
		const auto deltaMs = std::chrono::duration_cast<std::chrono::milliseconds>(nowPoint - m_lastUpdatePoint);
		m_client->Update(Unit::Time::Millisecond(deltaMs.count()));
		m_client->PostUpdate();
		m_lastUpdatePoint = nowPoint;

		std::this_thread::yield();
	}

	m_client.Reset();
	m_connectedHost.Reset();
	m_clientThreadStatus = ClientThreadStatus::Stopped;
}

void Client::ClientWorld::ProcessMessageFromHost(Host::MessageToClient& message)
{
	message.Match(
		[](Host::NotifyOfHostConnected_MessageToClient& hostConnectedPayload)
		{
			// TODO(network)
		},
		[this](Host::NotifyOfHostDisconnected_MessageToClient& hostDisconnectedPayload)
		{
			NotifyOfHostDisconnected();
		},
		[this](Host::ECSUpdate_MessageToClient& ecsUpdatePayload)
		{
			m_client->NotifyOfECSUpdateTransmission(ecsUpdatePayload.m_bytes);
		});
}

void Client::ClientWorld::ProcessInputMessage(Input::InputMessage& message)
{
	if (message.Is<Input::InputMessage_WindowClosed>())
	{
		m_connectedHost->Disconnect();
		return;
	}

	m_client->NotifyOfInputMessage(message);
}
