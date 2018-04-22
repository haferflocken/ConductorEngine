#include <host/ConnectedClient.h>

#include <host/MessageToClient.h>

#include <collection/LocklessQueue.h>

void Host::ConnectedClient::NotifyOfHostDisconnected()
{
	MessageToClient message;
	message.m_type = MessageToClientType::NotifyOfHostDisconnected;

	m_hostToClientMessages.TryPush(std::move(message));
}
