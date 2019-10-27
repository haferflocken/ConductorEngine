#pragma once

#include <collection/Vector.h>
#include <ecs/EntityManager.h>
#include <input/CallbackRegistry.h>
#include <input/InputStateManager.h>
#include <network/ECSReceiver.h>

#include <functional>

namespace Conductor { class GameData; }
namespace Input { struct InputMessage; }

namespace Client
{
class ConnectedHost;

class ClientInstance
{
protected:
	ConnectedHost& m_connectedHost;
	Input::CallbackRegistry m_inputCallbackRegistry{};
	Input::InputStateManager m_inputStateManager;
	ECS::EntityManager m_entityManager;
	Network::ECSReceiver m_ecsReceiver;

public:
	ClientInstance(const Conductor::GameData& gameData, ConnectedHost& connectedHost);

	Input::CallbackRegistry& GetInputCallbackRegistry() { return m_inputCallbackRegistry; }
	const Input::CallbackRegistry& GetInputCallbackRegistry() const { return m_inputCallbackRegistry; }

	ECS::EntityManager& GetEntityManager() { return m_entityManager; }
	const ECS::EntityManager& GetEntityManager() const { return m_entityManager; }

	void NotifyOfECSUpdateTransmission(const Collection::Vector<uint8_t>& transmissionBytes);
	void NotifyOfInputMessage(const Input::InputMessage& message);

	Collection::Vector<uint8_t> SerializeInputStateTransmission() const;

	void Update(const Unit::Time::Millisecond delta);

	void PostUpdate();
};
}
