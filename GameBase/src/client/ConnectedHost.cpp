#include <client/ConnectedHost.h>

#include <client/MessageToHost.h>

#include <collection/LocklessQueue.h>

void Client::ConnectedHost::Disconnect()
{
	MessageToHost message;
	message.m_clientID = m_clientID;
	message.m_type = Client::MessageToHostType::Disconnect;

	m_clientToHostMessages.TryPush(std::move(message));
}
