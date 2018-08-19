#include <client/IClient.h>

void Client::IClient::NotifyOfECSUpdateTransmission(const Collection::Vector<uint8_t>& transmissionBytes)
{
	m_entityManager.ApplyDeltaTransmission(transmissionBytes);
}
