#include <client/ClientWorld.h>

#include <client/ConnectedHost.h>
#include <client/IClient.h>
#include <client/IRenderInstance.h>
#include <input/InputMessage.h>

#include <collection/LocklessQueue.h>
#include <dev/Dev.h>
#include <host/MessageToClient.h>

Client::ClientWorld::ClientWorld(const Conductor::IGameData& gameData,
	IRenderInstance& renderInstance,
	Collection::LocklessQueue<Input::InputMessage>& inputMessages,
	Collection::LocklessQueue<Host::MessageToClient>& networkInputQueue,
	ClientFactory&& clientFactory)
	: m_gameData(gameData)
	, m_renderInstance(renderInstance)
	, m_inputMessages(inputMessages)
	, m_networkInputQueue(networkInputQueue)
	, m_clientFactory(std::move(clientFactory))
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
	m_renderInstance.InitOnClientThread();
	m_client = m_clientFactory(m_gameData, m_renderInstance.GetSceneViewFrustum(), *m_connectedHost);
	m_renderInstance.RegisterSystems(m_client->GetEntityManager());
	m_lastUpdatePoint = std::chrono::steady_clock::now();

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
	m_renderInstance.ShutdownOnClientThread();
	m_connectedHost.Reset();
	m_clientThreadStatus = ClientThreadStatus::Stopped;
}

void Client::ClientWorld::ProcessMessageFromHost(Host::MessageToClient& message)
{
	message.Match(
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
