#pragma once

#include <client/ClientID.h>
#include <collection/LocklessQueue.h>
#include <collection/Vector.h>
#include <host/MessageToClient.h>

namespace Client
{
struct MessageToHost;

// ConnectedHost defines an asynchronous interface which a client uses to send data to a host.
class ConnectedHost
{
	ClientID m_clientID;
	Collection::LocklessQueue<Client::MessageToHost>& m_clientToHostMessages;

public:
	ConnectedHost(ClientID clientID,
		Collection::LocklessQueue<Client::MessageToHost>& clientToHostMessages)
		: m_clientID(clientID)
		, m_clientToHostMessages(clientToHostMessages)
	{}

	ClientID GetClientID() const { return m_clientID; }

	// Connect to a host. In Client code, pass in null; in Host network code, pass in a non-null value.
	void Connect(Collection::LocklessQueue<Host::MessageToClient>* hostToClientMessages);

	void Disconnect();

	void TransmitFrameAcknowledgement(const uint64_t frameIndex);
	void TransmitInputStates(Collection::Vector<uint8_t>&& inputStatesBytes);
};
}
