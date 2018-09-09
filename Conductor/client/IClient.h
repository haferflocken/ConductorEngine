#pragma once

#include <collection/Vector.h>
#include <ecs/EntityManager.h>

namespace Client
{
class ConnectedHost;
struct InputMessage;

// IClient is the interface a game's client must implement.
class IClient
{
protected:
	ConnectedHost& m_connectedHost;
	ECS::EntityManager m_entityManager;

public:
	IClient(const ECS::ComponentReflector& componentReflector, ConnectedHost& connectedHost)
		: m_connectedHost(connectedHost)
		, m_entityManager(componentReflector, false)
	{}

	ECS::EntityManager& GetEntityManager() { return m_entityManager; }
	const ECS::EntityManager& GetEntityManager() const { return m_entityManager; }

	void NotifyOfECSUpdateTransmission(const Collection::Vector<uint8_t>& transmissionBytes);

	virtual void Update() = 0;

	virtual void NotifyOfWindowClosed() {};
	virtual void NotifyOfKeyUp(const char key) {};
	virtual void NotifyOfKeyDown(const char key) {};
	virtual void NotifyOfInputMessage(Client::InputMessage& message) {}
};
}
