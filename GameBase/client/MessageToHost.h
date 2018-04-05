#pragma once

#include <client/ClientID.h>

namespace Client
{
enum class MessageToHostType : uint8_t
{
	Disconnect = 0,
	Count
};

struct MessageToHost
{
	Client::ClientID m_clientID{ 0 };
	MessageToHostType m_type{ MessageToHostType::Count };
};
}
