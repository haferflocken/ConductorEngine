#include <host/IHost.h>

#include <client/ClientID.h>
#include <dev/Profiler.h>
#include <input/InputSystem.h>

namespace Host
{
IHost::IHost(Asset::AssetManager& assetManager, const ECS::ComponentReflector& componentReflector)
	// Entities and components the host creates begin their IDs at 0.
	: m_entityManager(assetManager, componentReflector, ECS::EntityID(0), 0)
	, m_ecsTransmitter()
	, m_inputSystem(m_entityManager.RegisterSystem(Mem::MakeUnique<Input::InputSystem>()))
{
}

void IHost::NotifyOfClientConnected(const Client::ClientID clientID, const Input::InputStateManager& inputStateManager)
{
	m_ecsTransmitter.NotifyOfClientConnected(clientID);
	m_inputSystem.AddClient(clientID, inputStateManager);
}

void IHost::NotifyOfClientDisconnected(const Client::ClientID clientID)
{
	m_inputSystem.RemoveClient(clientID);
	m_ecsTransmitter.NotifyOfClientDisconnected(clientID);
}

void IHost::NotifyOfFrameAcknowledgement(const Client::ClientID clientID, const uint64_t frameIndex)
{
	m_ecsTransmitter.NotifyOfFrameAcknowledgement(clientID, frameIndex);
}

void IHost::StoreECSFrame()
{
	AMP_PROFILE_SCOPE();

	ECS::SerializedEntitiesAndComponents serializedFrame;
	m_entityManager.FullySerializeAllEntitiesAndComponentsMatchingFilter(
		[](const ECS::Entity& entity)
		{
			return (entity.GetFlags() & ECS::EntityFlags::Networked) != ECS::EntityFlags::None;
		},
		serializedFrame);

	m_ecsTransmitter.AddSerializedFrame(std::move(serializedFrame));
}

void IHost::SerializeECSUpdateTransmission(
	const Client::ClientID clientID,
	Collection::Vector<uint8_t>& outTransmission)
{
	m_ecsTransmitter.TransmitFrame(clientID, outTransmission);
}
}
