#pragma once

#include <cstdint>

namespace Input
{
/**
 * An input source is an identifier for a key, mouse button, mouse axis, joystick axis, etc.
 * Only one keyboard and one mouse is supported per client.
 */
struct InputSource final
{
	using DeviceID = int32_t;
	using DeviceInputCode = int32_t;

	static constexpr DeviceID k_invalidDeviceID = 0;
	static constexpr DeviceID k_keyboardID = 1;
	static constexpr DeviceID k_mouseID = 2;
	static constexpr DeviceID k_touchID = 3;
	static constexpr DeviceID k_otherDeviceIDsBegin = 4;

	static constexpr DeviceInputCode k_mouseAxisX = 16;
	static constexpr DeviceInputCode k_mouseAxisY = 17;
	static constexpr DeviceInputCode k_mouseWheel = 18;

	// A device is a mouse, keyboard, gamepad, or similar device.
	DeviceID m_deviceID;
	// The code within the device that identifies the source. Can be a key code, a mouse button index, etc.
	DeviceInputCode m_deviceInputCode;

	bool operator<(const InputSource& rhs) const
	{
		if (m_deviceID < rhs.m_deviceID)
		{
			return true;
		}
		if (m_deviceID == rhs.m_deviceID)
		{
			return m_deviceInputCode < rhs.m_deviceInputCode;
		}
		return false;
	}

	bool operator==(const InputSource& rhs) const
	{
		return m_deviceID == rhs.m_deviceID && m_deviceInputCode == rhs.m_deviceInputCode;
	}
};

namespace Sources
{
constexpr InputSource Invalid{ InputSource::k_invalidDeviceID, 0 };

inline constexpr InputSource Key(const int32_t keyCode) { return { InputSource::k_keyboardID, keyCode }; }

inline constexpr InputSource MouseButton(const int32_t buttonIndex) { return { InputSource::k_mouseID, buttonIndex }; }
constexpr InputSource MouseX{ InputSource::k_mouseID, InputSource::k_mouseAxisX };
constexpr InputSource MouseY{ InputSource::k_mouseID, InputSource::k_mouseAxisY };
constexpr InputSource MouseWheel{ InputSource::k_mouseID, InputSource::k_mouseWheel };

constexpr InputSource ControllerButton(const int32_t controllerID, const uint8_t button) { return { controllerID, button }; }
constexpr InputSource ControllerAxis(const int32_t controllerID, const uint8_t axis) { return { controllerID, axis << 8 }; }
}
}
