#pragma once

#include <ecs/EntityManager.h>
#include <input/CallbackRegistry.h>

namespace Client { struct ClientID; }

namespace Host
{
// IHost is the interface a game's host must implement.
class IHost
{
protected:
	Input::CallbackRegistry m_inputCallbackRegistry;
	ECS::EntityManager m_entityManager;

public:
	IHost(Asset::AssetManager& assetManager, const ECS::ComponentReflector& componentReflector)
		: m_entityManager(assetManager, componentReflector, true)
	{}

	Collection::Vector<uint8_t> SerializeECSUpdateTransmission();

	void NotifyOfInputMessage(const Client::ClientID clientID, const Input::InputMessage& message);

	virtual void Update(const Unit::Time::Millisecond delta) = 0;
};
}
