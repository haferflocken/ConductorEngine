#include <client/IClient.h>

#include <client/ConnectedHost.h>
#include <input/InputMessage.h>
#include <input/InputSystem.h>

namespace Client
{
IClient::IClient(Asset::AssetManager& assetManager,
	const ECS::ComponentReflector& componentReflector,
	ConnectedHost& connectedHost)
	: m_connectedHost(connectedHost)
	, m_inputStateManager(m_inputCallbackRegistry)
	, m_entityManager(assetManager, componentReflector, false)
{
	// The InputSystem is present for all clients.
	Input::InputSystem& inputSystem = m_entityManager.RegisterSystem(Mem::MakeUnique<Input::InputSystem>());
	inputSystem.AddClient(m_connectedHost.GetClientID(), m_inputStateManager);
}

void IClient::NotifyOfECSUpdateTransmission(const Collection::Vector<uint8_t>& transmissionBytes)
{
	m_entityManager.ApplyDeltaTransmission(transmissionBytes);
}

void IClient::NotifyOfInputMessage(const Input::InputMessage& message)
{
	m_inputCallbackRegistry.NotifyOfInputMessage(message);
}

void IClient::PostUpdate()
{
	m_inputStateManager.ResetInputStates();
}
}
