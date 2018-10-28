#include <input/InputSystem.h>

#include <input/CallbackRegistry.h>

namespace Input
{
InputSystem::InputSystem(Input::CallbackRegistry& callbackRegistry)
{
	callbackRegistry.RegisterInputCallback<>(
		[this](const Client::ClientID clientID, const InputMessage& message)
		{
			NotifyOfInputMessage(clientID, message);
		});
}

void InputSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	for (const auto& ecsGroup : ecsGroups)
	{
		auto& inputComponent = ecsGroup.Get<InputComponent>();
		auto& inputMap = inputComponent.m_inputMap;

		const Collection::VectorMap<InputSource, InputStateBuffer>& clientInputs =
			m_inputsPerClient[inputComponent.m_clientID];

		for (const auto& entry : clientInputs)
		{
			const InputSource& source = entry.first;
			const InputStateBuffer& stateBuffer = entry.second;

			// If the component uses the input, let it know of the buffered states.
			auto entry = inputMap.Find(source);
			if (entry != inputMap.end())
			{
				entry->second = stateBuffer;
			}
		}
	}

	for (auto& entry : m_inputsPerClient)
	{
		Collection::VectorMap<InputSource, InputStateBuffer>& clientInputs = entry.second;
		clientInputs.Clear();
	}
}

void InputSystem::NotifyOfInputMessage(const Client::ClientID clientID, const InputMessage& message)
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

	Collection::VectorMap<InputSource, InputStateBuffer>& clientInputs = m_inputsPerClient[clientID];
	for (size_t i = 0; i < k_maxSources; ++i)
	{
		if (sources[i].m_deviceID == InputSource::k_invalidDeviceID)
		{
			break;
		}
		InputStateBuffer& stateBuffer = clientInputs[sources[i]];
		if (stateBuffer.m_count < InputStateBuffer::k_capacity)
		{
			stateBuffer.m_values[stateBuffer.m_count++] = inputValues[i];
		}
	}
}
}
