#include <input/InputSystem.h>

#include <input/CallbackRegistry.h>

namespace Input
{
InputSystem::InputSystem(Input::CallbackRegistry& callbackRegistry)
{
	callbackRegistry.RegisterInputCallback<>([this](const InputMessage& message) { NotifyOfInputMessage(message); });
}

void InputSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	// TODO(input) InputSystem
}

void InputSystem::NotifyOfInputMessage(const InputMessage& message)
{
	// TODO(input) InputSystem
}
}
