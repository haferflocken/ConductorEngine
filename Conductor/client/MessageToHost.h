#pragma once

#include <client/ClientID.h>
#include <host/MessageToClient.h>

namespace Collection { template <typename T> class LocklessQueue; }

namespace Client
{
enum class MessageToHostType : uint8_t
{
	Connect = 0,
	Disconnect,
	Count
};

struct ConnectMessageToHost
{
	Collection::LocklessQueue<Host::MessageToClient>* m_hostToClientMessages{ nullptr };
};

struct MessageToHost
{
	MessageToHost() {}

	Client::ClientID m_clientID{ 0 };
	MessageToHostType m_type{ MessageToHostType::Count };
	union
	{
		ConnectMessageToHost m_connectPayload;
	};
};
}
