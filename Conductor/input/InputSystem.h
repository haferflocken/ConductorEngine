#pragma once

#include <client/ClientID.h>
#include <collection/VectorMap.h>
#include <ecs/System.h>
#include <input/InputComponent.h>

namespace Input
{
class CallbackRegistry;
struct InputMessage;

/**
 * The InputSystem processes user input and stores it in InputComponents so that entities have access to it.
 */
class InputSystem final : public ECS::SystemTempl<Util::TypeList<>, Util::TypeList<InputComponent>>
{
public:
	explicit InputSystem(Input::CallbackRegistry& callbackRegistry);

	virtual ~InputSystem() {}

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

private:
	void NotifyOfInputMessage(const Client::ClientID clientID, const InputMessage& message);

	Collection::VectorMap<Client::ClientID, Collection::VectorMap<InputSource, InputStateBuffer>> m_inputsPerClient;
};
}
