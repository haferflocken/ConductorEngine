#include <client/IClient.h>

#include <client/ConnectedHost.h>
#include <input/InputMessage.h>

namespace Client
{
void IClient::NotifyOfECSUpdateTransmission(const Collection::Vector<uint8_t>& transmissionBytes)
{
	m_entityManager.ApplyDeltaTransmission(transmissionBytes);
}

void IClient::NotifyOfInputMessage(const Input::InputMessage& message)
{
	m_inputCallbackRegistry.NotifyOfInputMessage(m_connectedHost.GetClientID(), message);
}
}
