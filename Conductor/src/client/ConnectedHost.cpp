#include <client/ConnectedHost.h>

#include <client/MessageToHost.h>

namespace Client
{
void ConnectedHost::Connect(Collection::LocklessQueue<Host::MessageToClient>* hostToClientMessages)
{
	auto message = MessageToHost::Make<MessageToHost_Connect>(m_clientID);
	auto& payload = message.Get<MessageToHost_Connect>();
	payload.m_hostToClientMessages = hostToClientMessages;

	m_clientToHostMessages.TryPush(std::move(message));
}

void ConnectedHost::Disconnect()
{
	auto message = MessageToHost::Make<MessageToHost_Disconnect>(m_clientID);
	m_clientToHostMessages.TryPush(std::move(message));
}

void ConnectedHost::TransmitInputStates(Collection::Vector<uint8_t>&& inputStatesBytes)
{
	auto message = MessageToHost::Make<MessageToHost_InputStates>(m_clientID);
	auto& payload = message.Get<MessageToHost_InputStates>();
	payload.m_bytes = std::move(inputStatesBytes);

	m_clientToHostMessages.TryPush(std::move(message));
}
}
