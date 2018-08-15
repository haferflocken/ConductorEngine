#include <host/ConnectedClient.h>

#include <host/MessageToClient.h>

#include <collection/LocklessQueue.h>

void Host::ConnectedClient::NotifyOfHostDisconnected()
{
	MessageToClient message = MessageToClient::Make<NotifyOfHostDisconnected_MessageToClient>();
	
	m_hostToClientMessages.TryPush(std::move(message));
}
