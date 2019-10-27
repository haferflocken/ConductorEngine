#pragma once

#include <ecs/EntityManager.h>
#include <network/ECSTransmitter.h>

namespace Client { struct ClientID; }
namespace Conductor { class GameData; }

namespace Input
{
class InputStateManager;
class InputSystem;
}

namespace Host
{
class HostInstance
{
protected:
	ECS::EntityManager m_entityManager;
	Network::ECSTransmitter m_ecsTransmitter;
	// The InputSystem is present on all hosts.
	Input::InputSystem& m_inputSystem;

public:
	HostInstance(const Conductor::GameData& gameData);

	void NotifyOfClientConnected(const Client::ClientID clientID, const Input::InputStateManager& inputStateManager);
	void NotifyOfClientDisconnected(const Client::ClientID clientID);
	void NotifyOfFrameAcknowledgement(const Client::ClientID clientID, const uint64_t frameIndex);

	void StoreECSFrame();
	void SerializeECSUpdateTransmission(const Client::ClientID clientID, Collection::Vector<uint8_t>& outTransmission);

	void Update(const Unit::Time::Millisecond delta);
};
}
