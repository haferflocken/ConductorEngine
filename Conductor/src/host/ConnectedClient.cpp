#include <host/ConnectedClient.h>

#include <host/MessageToClient.h>

#include <collection/LocklessQueue.h>

namespace Host
{
void ConnectedClient::TransmitHostDisconnectedNotification()
{
	auto message = MessageToClient::Make<NotifyOfHostDisconnected_MessageToClient>();
	m_hostToClientMessages.TryPush(std::move(message));
}

void ConnectedClient::TransmitECSUpdate(const Collection::Vector<uint8_t>& transmissionBytes)
{
	MessageToClient message = MessageToClient::Make<ECSUpdate_MessageToClient>();
	auto& ecsUpdateMessage = message.Get<ECSUpdate_MessageToClient>();

	ecsUpdateMessage.m_bytes.Resize(transmissionBytes.Size());
	memcpy(&ecsUpdateMessage.m_bytes.Front(), &transmissionBytes.Front(), transmissionBytes.Size());

	m_hostToClientMessages.TryPush(std::move(message));
}

void ConnectedClient::NotifyOfInputStatesTransmission(const Collection::Vector<uint8_t>& transmissionBytes)
{
	m_clientInputStateManager.ApplyFullTransmission(transmissionBytes);
}
}
