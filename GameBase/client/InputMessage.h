#pragma once

#include <cstdint>

namespace Client
{
enum class InputMessageType : uint8_t
{
	WindowClosed = 0,
	KeyUp,
	KeyDown,
	Count
};

struct InputMessage
{
	InputMessageType m_type;
	union
	{
		char m_key;
	};
};
}
