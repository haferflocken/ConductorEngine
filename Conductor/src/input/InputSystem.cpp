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
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	// TODO(input) InputSystem
}

void InputSystem::NotifyOfInputMessage(const Client::ClientID clientID, const InputMessage& message)
{
	// TODO(input) InputSystem
}
}
