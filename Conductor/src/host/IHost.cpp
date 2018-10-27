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
	// TODO(host) Separate inputs from different clients
	m_inputCallbackRegistry.NotifyOfInputMessage(message);
}
}
