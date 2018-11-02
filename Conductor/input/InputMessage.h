#pragma once

#include <collection/Variant.h>

#include <cstdint>

namespace Input
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

struct InputMessage_MouseMotion final
{
	// The mouse position from (0, 0) to (1, 1).
	float m_mouseX;
	float m_mouseY;
};

struct InputMessage_MouseButtonUp final
{
	// The mouse position from (0, 0) to (1, 1).
	float m_mouseX;
	float m_mouseY;
	uint8_t m_buttonIndex;
};

struct InputMessage_MouseButtonDown final
{
	// The mouse position from (0, 0) to (1, 1).
	float m_mouseX;
	float m_mouseY;
	uint8_t m_buttonIndex;
};

struct InputMessage_MouseWheel final
{
	// TODO
};

struct InputMessage_ControllerAxisMotion final
{
	int32_t m_controllerID;
	int16_t m_value;
	uint8_t m_axis;
};

struct InputMessage_ControllerButtonUp final
{
	int32_t m_controllerID;
	uint8_t m_button;
};

struct InputMessage_ControllerButtonDown final
{
	int32_t m_controllerID;
	uint8_t m_button;
};

struct InputMessage_ControllerAdded final
{
	int32_t m_controllerID;
};

struct InputMessage_ControllerRemoved final
{
	int32_t m_controllerID;
};

struct InputMessage final : public Collection::Variant<
	InputMessage_WindowClosed,
	InputMessage_KeyUp,
	InputMessage_KeyDown,
	InputMessage_MouseMotion,
	InputMessage_MouseButtonUp,
	InputMessage_MouseButtonDown,
	InputMessage_MouseWheel,
	InputMessage_ControllerAxisMotion,
	InputMessage_ControllerButtonUp,
	InputMessage_ControllerButtonDown,
	InputMessage_ControllerAdded,
	InputMessage_ControllerRemoved>
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
