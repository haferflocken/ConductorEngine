#include <host/IHost.h>

#include <client/ClientID.h>

namespace Host
{
Collection::Vector<uint8_t> IHost::SerializeECSUpdateTransmission()
{
	return m_entityManager.SerializeDeltaTransmission();
}

void IHost::NotifyOfInputMessage(const Client::ClientID clientID, const Input::InputMessage& message)
{
	m_inputCallbackRegistry.NotifyOfInputMessage(clientID, message);
}
}
