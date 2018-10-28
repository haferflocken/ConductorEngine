#include <input/InputStateManager.h>

#include <input/CallbackRegistry.h>
#include <input/InputMessage.h>

namespace Input
{
InputStateManager::InputStateManager()
	: m_inputStateBuffers()
	, m_namedInputMapping()
{}

InputStateManager::InputStateManager(CallbackRegistry& callbackRegistry)
	: m_inputStateBuffers()
	, m_namedInputMapping()
{
	callbackRegistry.RegisterInputCallback<>(
		[this](const Client::ClientID, const InputMessage& message)
		{
			NotifyOfInputMessage(message);
		});
}

const InputStateBuffer* InputStateManager::FindInput(const InputSource inputSource) const
{
	const auto iter = m_inputStateBuffers.Find(inputSource);
	return (iter != m_inputStateBuffers.end()) ? &iter->second : nullptr;
}

const InputStateBuffer* InputStateManager::FindNamedInput(const Util::StringHash nameHash) const
{
	const auto iter = m_namedInputMapping.Find(nameHash);
	return (iter != m_namedInputMapping.end()) ? FindInput(iter->second) : nullptr;
}

void InputStateManager::ResetInputStates()
{
	for (auto& entry : m_inputStateBuffers)
	{
		entry.second.m_count = 0;
	}
}

void InputStateManager::NotifyOfInputMessage(const InputMessage& message)
{
	static constexpr size_t k_maxSources = 2;
	InputSource sources[k_maxSources]{
		InputSource{ InputSource::k_invalidDeviceID },
		InputSource{ InputSource::k_invalidDeviceID } };
	float inputValues[k_maxSources]{ -1.0f, -1.0f };

	// Transform the message to inputs.
	message.Match(
		[](const InputMessage_WindowClosed&) {},
		[&](const InputMessage_KeyUp& keyUp)
		{
			sources[0] = { InputSource::k_keyboardID, keyUp.m_keyCode };
			inputValues[0] = 0.0f;
		},
		[&](const InputMessage_KeyDown& keyDown)
		{
			sources[0] = { InputSource::k_keyboardID, keyDown.m_keyCode };
			inputValues[0] = 1.0f;
		},
		[&](const InputMessage_MouseMotion& mouseMotion)
		{
			sources[0] = { InputSource::k_mouseID, InputSource::k_mouseAxisX };
			inputValues[0] = mouseMotion.m_mouseX;

			sources[1] = { InputSource::k_mouseID, InputSource::k_mouseAxisY };
			inputValues[1] = mouseMotion.m_mouseY;
		},
		[&](const InputMessage_MouseButtonUp& mouseUp)
		{
			sources[0] = { InputSource::k_mouseID, mouseUp.m_buttonIndex };
			inputValues[0] = 0.0f;
		},
		[&](const InputMessage_MouseButtonDown& mouseDown)
		{
			sources[0] = { InputSource::k_mouseID, mouseDown.m_buttonIndex };
			inputValues[0] = 1.0f;
		});

	for (size_t i = 0; i < k_maxSources; ++i)
	{
		if (sources[i].m_deviceID == InputSource::k_invalidDeviceID)
		{
			break;
		}
		InputStateBuffer& stateBuffer = m_inputStateBuffers[sources[i]];
		if (stateBuffer.m_count < InputStateBuffer::k_capacity)
		{
			stateBuffer.m_values[stateBuffer.m_count++] = inputValues[i];
		}
	}
}
}
