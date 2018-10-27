#pragma once

#include <ecs/System.h>
#include <input/InputComponent.h>

namespace Client { class IClient; }

namespace Input
{
struct InputMessage;

/**
 * The InputSystem forwards client input to InputComponents so that entities have access to it.
 */
class InputSystem final : public ECS::SystemTempl<Util::TypeList<>, Util::TypeList<InputComponent>>
{
public:
	explicit InputSystem(Client::IClient& client);
	virtual ~InputSystem() {}

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const;

private:
	void NotifyOfInputMessage(const InputMessage& message);
};
}
