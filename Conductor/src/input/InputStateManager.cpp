#include <input/InputStateManager.h>

#include <input/CallbackRegistry.h>
#include <input/InputMessage.h>
#include <mem/Serialize.h>

namespace Input
{
InputStateManager::InputStateManager()
	: m_namedInputStateBuffers()
	, m_namedInputMapping()
{}

InputStateManager::InputStateManager(CallbackRegistry& callbackRegistry)
	: m_namedInputStateBuffers()
	, m_namedInputMapping()
{
	callbackRegistry.RegisterInputCallback<>([this](const InputMessage& message) { NotifyOfInputMessage(message); });
}

void InputStateManager::SetInputName(const InputSource source, const Util::StringHash nameHash)
{
	m_namedInputMapping.RemoveAllMatching([&](const auto& entry) { return entry.second == nameHash; });
	m_namedInputMapping[source] = nameHash;
}

const InputStateBuffer* InputStateManager::FindInput(const InputSource inputSource) const
{
	const auto iter = m_namedInputMapping.Find(inputSource);
	return (iter != m_namedInputMapping.end()) ? FindNamedInput(iter->second) : nullptr;
}

const InputStateBuffer* InputStateManager::FindNamedInput(const Util::StringHash nameHash) const
{
	const auto iter = m_namedInputStateBuffers.Find(nameHash);
	return (iter != m_namedInputStateBuffers.end()) ? &iter->second : nullptr;
}

void InputStateManager::ResetInputStates()
{
	for (auto& entry : m_namedInputStateBuffers)
	{
		InputStateBuffer& inputStateBuffer = entry.second;
		if (inputStateBuffer.m_count > 0)
		{
			inputStateBuffer.m_values[0] = inputStateBuffer.m_values[inputStateBuffer.m_count - 1];
			inputStateBuffer.m_count = 1;
		}
	}
}

Collection::Vector<uint8_t> InputStateManager::SerializeFullTransmission() const
{
	Collection::Vector<uint8_t> outBytes;

	const uint32_t numInputs = m_namedInputStateBuffers.Size();
	Mem::Serialize(numInputs, outBytes);

	for (const auto& entry : m_namedInputStateBuffers)
	{
		const char* const inputName = Util::ReverseHash(entry.first);
		const InputStateBuffer& inputStateBuffer = entry.second;

		Mem::Serialize(inputName, outBytes);
		outBytes.AddAll({ reinterpret_cast<const uint8_t*>(&inputStateBuffer), sizeof(InputStateBuffer) });
	}

	return outBytes;
}

void InputStateManager::ApplyFullTransmission(const Collection::Vector<uint8_t>& transmissionBytes)
{
	m_namedInputStateBuffers.Clear();

	const uint8_t* iter = transmissionBytes.begin();
	const uint8_t* const iterEnd = transmissionBytes.end();

	const auto maybeNumInputs = Mem::DeserializeUi32(iter, iterEnd);
	if (!maybeNumInputs.second)
	{
		return;
	}
	const size_t numInputs = maybeNumInputs.first;

	for (size_t i = 0; i < numInputs; ++i)
	{
		char nameBuffer[64];
		if (!Mem::DeserializeString(iter, iterEnd, nameBuffer))
		{
			return;
		}
		const Util::StringHash nameHash = Util::CalcHash(nameBuffer);

		if ((iter + sizeof(InputStateBuffer)) > iterEnd)
		{
			return;
		}
		InputStateBuffer& inputStateBuffer = m_namedInputStateBuffers[nameHash];
		memcpy(&inputStateBuffer, iter, sizeof(InputStateBuffer));

		// Clamp the buffer's count to its capacity in case the transmission is bad.
		if (inputStateBuffer.m_count > InputStateBuffer::k_capacity)
		{
			inputStateBuffer.m_count = InputStateBuffer::k_capacity;
		}

		iter += sizeof(InputStateBuffer);
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
			sources[0] = Sources::Key(keyUp.m_keyCode);
			inputValues[0] = 0.0f;
		},
		[&](const InputMessage_KeyDown& keyDown)
		{
			sources[0] = Sources::Key(keyDown.m_keyCode);
			inputValues[0] = 1.0f;
		},
		[&](const InputMessage_MouseMotion& mouseMotion)
		{
			sources[0] = Sources::MouseX;
			inputValues[0] = mouseMotion.m_mouseX;

			sources[1] = Sources::MouseY;
			inputValues[1] = mouseMotion.m_mouseY;
		},
		[&](const InputMessage_MouseButtonUp& mouseUp)
		{
			sources[0] = Sources::MouseButton(mouseUp.m_buttonIndex);
			inputValues[0] = 0.0f;
		},
		[&](const InputMessage_MouseButtonDown& mouseDown)
		{
			sources[0] = Sources::MouseButton(mouseDown.m_buttonIndex);
			inputValues[0] = 1.0f;
		},
		[&](const InputMessage_MouseWheel& mouseWheel)
		{
			// TODO
		},
		[&](const InputMessage_ControllerAxisMotion& controllerAxis)
		{
			sources[0] = Sources::ControllerAxis(controllerAxis.m_controllerID, controllerAxis.m_axis);
			inputValues[0] = static_cast<float>(controllerAxis.m_value) / INT16_MAX;
		},
		[&](const InputMessage_ControllerButtonUp& controllerButtonUp)
		{
			sources[0] = Sources::ControllerButton(controllerButtonUp.m_controllerID, controllerButtonUp.m_button);
			inputValues[0] = 0.0f;
		},
		[&](const InputMessage_ControllerButtonDown& controllerButtonDown)
		{
			sources[0] = Sources::ControllerButton(controllerButtonDown.m_controllerID, controllerButtonDown.m_button);
			inputValues[0] = 1.0f;
		}, 
		[](const InputMessage_ControllerAdded& controllerAdded) {},
		[&](const InputMessage_ControllerRemoved& controllerRemoved)
		{
			m_namedInputStateBuffers.RemoveAllMatching(
				[&](const auto& entry) -> bool
				{
					for (const auto& nameEntry : m_namedInputMapping)
					{
						if (nameEntry.first.m_deviceID == controllerRemoved.m_controllerID)
						{
							return true;
						}
					}
					return false;
				});
		});

	// Map the inputs to names and store them.
	for (size_t i = 0; i < k_maxSources; ++i)
	{
		if (sources[i].m_deviceID == InputSource::k_invalidDeviceID)
		{
			break;
		}

		const auto iter = m_namedInputMapping.Find(sources[i]);
		if (iter == m_namedInputMapping.end())
		{
			continue;
		}
		const Util::StringHash nameHash = iter->second;

		InputStateBuffer& stateBuffer = m_namedInputStateBuffers[nameHash];
		if (stateBuffer.m_count < InputStateBuffer::k_capacity)
		{
			stateBuffer.m_values[stateBuffer.m_count++] = inputValues[i];
		}
	}
}
}
