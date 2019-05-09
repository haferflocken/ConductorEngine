#include <host/IHost.h>

#include <client/ClientID.h>
#include <input/InputSystem.h>

namespace Host
{
IHost::IHost(Asset::AssetManager& assetManager, const ECS::ComponentReflector& componentReflector)
	: m_entityManager(assetManager, componentReflector)
	, m_entityTransmitter()
	, m_inputSystem(m_entityManager.RegisterSystem(Mem::MakeUnique<Input::InputSystem>()))
{
}

void IHost::NotifyOfClientConnected(const Client::ClientID clientID, const Input::InputStateManager& inputStateManager)
{
	m_entityTransmitter.NotifyOfClientConnected(clientID);
	m_inputSystem.AddClient(clientID, inputStateManager);
}

void IHost::NotifyOfClientDisconnected(const Client::ClientID clientID)
{
	m_inputSystem.RemoveClient(clientID);
	m_entityTransmitter.NotifyOfClientDisconnected(clientID);
}

void IHost::StoreECSFrame()
{
	ECS::SerializedEntitiesAndComponents serializedFrame;
	m_entityManager.FullySerializeAllEntitiesAndComponents(serializedFrame);
	m_entityTransmitter.AddSerializedFrame(std::move(serializedFrame));
}

void IHost::SerializeECSUpdateTransmission(
	const Client::ClientID clientID,
	Collection::Vector<uint8_t>& outTransmission)
{
	m_entityTransmitter.TransmitFrame(clientID, outTransmission);
}
}
