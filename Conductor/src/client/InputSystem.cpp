#include <client/InputSystem.h>

#include <client/IClient.h>

namespace Client
{
InputSystem::InputSystem(IClient& client)
{
	client.RegisterInputCallback<>([this](const InputMessage& message) { NotifyOfInputMessage(message); });
}

void InputSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	// TODO(client) InputSystem
}

void InputSystem::NotifyOfInputMessage(const InputMessage& message)
{
	// TODO(client) InputSystem
}
}
