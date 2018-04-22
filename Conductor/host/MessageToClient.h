#pragma once

#include <cstdint>

namespace Host
{
enum class MessageToClientType : uint8_t
{
	NotifyOfHostDisconnected = 0,
	Count
};

struct MessageToClient
{
	MessageToClientType m_type{ MessageToClientType::Count };
};
}
