#pragma once

#include <client/ClientID.h>
#include <collection/VectorMap.h>
#include <ecs/System.h>
#include <input/InputComponent.h>

namespace Input
{
class InputStateManager;

/**
 * The InputSystem processes user input and stores it in InputComponents so that entities have access to it.
 */
class InputSystem final : public ECS::SystemTempl<Util::TypeList<>, Util::TypeList<InputComponent>>
{
public:
	InputSystem() = default;
	virtual ~InputSystem() {}

	void AddClient(const Client::ClientID clientID, const Input::InputStateManager& inputStateManager);
	void RemoveClient(const Client::ClientID clientID);

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

private:
	Collection::VectorMap<Client::ClientID, const Input::InputStateManager*> m_inputStateManagersPerClient;
};
}
