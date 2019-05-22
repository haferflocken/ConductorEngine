#include <client/IClient.h>

#include <client/ConnectedHost.h>
#include <dev/Profiler.h>
#include <input/InputMessage.h>
#include <input/InputSystem.h>

namespace Client
{
IClient::IClient(Asset::AssetManager& assetManager,
	const ECS::ComponentReflector& componentReflector,
	ConnectedHost& connectedHost)
	: m_connectedHost(connectedHost)
	, m_inputStateManager(m_inputCallbackRegistry)
	// Entities and components the client creates have their high bit set.
	, m_entityManager(assetManager, componentReflector, ECS::EntityID(1ui32 << 31), 1ui64 << 63)
	, m_ecsReceiver()
{
	// The InputSystem is present for all clients.
	Input::InputSystem& inputSystem = m_entityManager.RegisterSystem(Mem::MakeUnique<Input::InputSystem>());
	inputSystem.AddClient(m_connectedHost.GetClientID(), m_inputStateManager);
}

void IClient::NotifyOfECSUpdateTransmission(const Collection::Vector<uint8_t>& transmissionBytes)
{
	AMP_PROFILE_SCOPE();

	const ECS::SerializedEntitiesAndComponents* const newFrame =
		m_ecsReceiver.TryReceiveFrameTransmission(transmissionBytes.GetConstView());
	if (newFrame != nullptr)
	{
		m_entityManager.SetNetworkedEntitiesToFullSerialization(*newFrame);
		m_connectedHost.TransmitFrameAcknowledgement(m_ecsReceiver.GetLastSeenFrameIndex());
	}
}

void IClient::NotifyOfInputMessage(const Input::InputMessage& message)
{
	m_inputCallbackRegistry.NotifyOfInputMessage(message);
}

Collection::Vector<uint8_t> IClient::SerializeInputStateTransmission() const
{
	return m_inputStateManager.SerializeFullTransmission();
}

void IClient::PostUpdate()
{
	m_inputStateManager.ResetInputStates();
}
}
