#pragma once

#include <cstdint>

namespace Client
{
enum class MessageToHostType : uint8_t
{
	Disconnect = 0,
	Count
};

struct MessageToHost
{
	uint16_t m_clientID{ 0 };
	MessageToHostType m_type{ MessageToHostType::Count };
};
}
