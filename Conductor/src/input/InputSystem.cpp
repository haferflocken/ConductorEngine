#include <input/InputSystem.h>

#include <input/InputStateManager.h>

namespace Input
{
void InputSystem::AddClient(const Client::ClientID clientID, const Input::InputStateManager& inputStateManager)
{
	AMP_ASSERT(m_inputStateManagersPerClient.Find(clientID) == m_inputStateManagersPerClient.end(),
		"A client should not be added to the InputSystem multiple times!");
	m_inputStateManagersPerClient[clientID] = &inputStateManager;
}

void InputSystem::RemoveClient(const Client::ClientID clientID)
{
	[[maybe_unused]] const bool isRemoved = m_inputStateManagersPerClient.TryRemove(clientID);
	AMP_ASSERT(isRemoved, "Failed to remove client from the InputSystem!");
}

void InputSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	for (const auto& ecsGroup : ecsGroups)
	{
		auto& inputComponent = ecsGroup.Get<InputComponent>();
		auto& inputMap = inputComponent.m_inputMap;

		// Clear the component's state buffers.
		for (auto& entry : inputMap)
		{
			entry.second.m_count = 0;
		}
		
		// Copy to the component any inputs it reads.
		const auto clientInputManagerIter = m_inputStateManagersPerClient.Find(inputComponent.m_clientID);
		if (clientInputManagerIter == m_inputStateManagersPerClient.end())
		{
			continue;
		}
		const InputStateManager& clientInputManager = *clientInputManagerIter->second;

		for (auto& entry : inputMap)
		{
			const InputSource& source = entry.first;
			InputStateBuffer& stateBuffer = entry.second;

			const InputStateBuffer* const clientInputBuffer = clientInputManager.FindInput(source);
			if (clientInputBuffer != nullptr)
			{
				stateBuffer = *clientInputBuffer;
			}
		}
	}
}
}
