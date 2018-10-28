#pragma once

#include <collection/Vector.h>
#include <ecs/EntityManager.h>
#include <input/CallbackRegistry.h>
#include <input/InputStateManager.h>

#include <functional>

namespace Input { struct InputMessage; }

namespace Client
{
class ConnectedHost;

// IClient is the interface a game's client must implement.
class IClient
{
protected:
	ConnectedHost& m_connectedHost;
	Input::CallbackRegistry m_inputCallbackRegistry{};
	Input::InputStateManager m_inputStateManager;
	ECS::EntityManager m_entityManager;

public:
	IClient(Asset::AssetManager& assetManager, const ECS::ComponentReflector& componentReflector,
		ConnectedHost& connectedHost);

	ECS::EntityManager& GetEntityManager() { return m_entityManager; }
	const ECS::EntityManager& GetEntityManager() const { return m_entityManager; }

	void NotifyOfECSUpdateTransmission(const Collection::Vector<uint8_t>& transmissionBytes);
	void NotifyOfInputMessage(const Input::InputMessage& message);

	Collection::Vector<uint8_t> SerializeInputStateTransmission() const;

	virtual void Update(const Unit::Time::Millisecond delta) = 0;

	void PostUpdate();
};
}
