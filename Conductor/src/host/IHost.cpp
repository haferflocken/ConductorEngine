#include <host/IHost.h>

#include <client/ClientID.h>
#include <input/InputSystem.h>

namespace Host
{
IHost::IHost(Asset::AssetManager& assetManager, const ECS::ComponentReflector& componentReflector)
	: m_entityManager(assetManager, componentReflector, true)
	, m_inputSystem(m_entityManager.RegisterSystem(Mem::MakeUnique<Input::InputSystem>()))
{
}

void IHost::NotifyOfClientConnected(const Client::ClientID clientID, const Input::InputStateManager& inputStateManager)
{
	m_inputSystem.AddClient(clientID, inputStateManager);
}

void IHost::NotifyOfClientDisconnected(const Client::ClientID clientID)
{
	m_inputSystem.RemoveClient(clientID);
}

Collection::Vector<uint8_t> IHost::SerializeECSUpdateTransmission()
{
	return m_entityManager.SerializeDeltaTransmission();
}
}
