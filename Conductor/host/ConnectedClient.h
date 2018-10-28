#pragma once

#include <client/ClientID.h>
#include <host/MessageToClient.h>
#include <input/InputStateManager.h>

namespace Collection { template <typename T> class LocklessQueue; }

namespace Host
{
/**
 * ConnectedClient defines an asynchronous interface which a host uses to send data to a client
 * and to store client input states.
 */
class ConnectedClient
{
	Client::ClientID m_clientID;
	Input::InputStateManager m_clientInputStateManager;
	Collection::LocklessQueue<Host::MessageToClient>& m_hostToClientMessages;

public:
	ConnectedClient(Client::ClientID clientID,
		Collection::LocklessQueue<Host::MessageToClient>& hostToClientMessages)
			: m_clientID(clientID)
			, m_clientInputStateManager()
			, m_hostToClientMessages(hostToClientMessages)
		{}

	Client::ClientID GetClientID() const { return m_clientID; }
	const Input::InputStateManager GetClientInputStateManager() const { return m_clientInputStateManager; }

	void NotifyOfHostDisconnected();
	void TransmitECSUpdate(const Collection::Vector<uint8_t>& transmissionBytes);
};
}
