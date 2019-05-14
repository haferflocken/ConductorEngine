#pragma once

#include <ecs/EntityManager.h>
#include <network/ECSTransmitter.h>

namespace Client { struct ClientID; }

namespace Input
{
class InputStateManager;
class InputSystem;
}

namespace Host
{
/**
 * IHost is the interface a game's host must implement.
 */
class IHost
{
protected:
	ECS::EntityManager m_entityManager;
	Network::ECSTransmitter m_ecsTransmitter;
	// The InputSystem is present on all hosts.
	Input::InputSystem& m_inputSystem;

public:
	IHost(Asset::AssetManager& assetManager, const ECS::ComponentReflector& componentReflector);

	void NotifyOfClientConnected(const Client::ClientID clientID, const Input::InputStateManager& inputStateManager);
	void NotifyOfClientDisconnected(const Client::ClientID clientID);

	void StoreECSFrame();
	void SerializeECSUpdateTransmission(const Client::ClientID clientID, Collection::Vector<uint8_t>& outTransmission);

	virtual void Update(const Unit::Time::Millisecond delta) = 0;
};
}
