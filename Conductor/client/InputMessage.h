#pragma once

#include <collection/Variant.h>

#include <cstdint>

namespace Client
{
struct InputMessage_WindowClosed final {};

struct InputMessage_KeyUp final
{
	int32_t m_keyCode;
	char m_keyName[28];
};

struct InputMessage_KeyDown final
{
	int32_t m_keyCode;
	char m_keyName[28];
};

struct InputMessage final : public Collection::Variant<
	InputMessage_WindowClosed,
	InputMessage_KeyUp,
	InputMessage_KeyDown>
{
	using Variant::Variant;

	InputMessage(Variant&& v)
		: Variant(std::move(v))
	{}

	template <typename T, typename... Args>
	static InputMessage Make(Args&&... args)
	{
		return InputMessage(Variant::Make<T, Args...>(std::forward<Args>(args)...));
	}
};
}
