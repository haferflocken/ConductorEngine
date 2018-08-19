#pragma once

#include <ecs/EntityManager.h>

namespace Host
{
// IHost is the interface a game's host must implement.
class IHost
{
protected:
	ECS::EntityManager m_entityManager;

public:
	explicit IHost(const ECS::ComponentReflector& componentReflector)
		: m_entityManager(componentReflector, true)
	{}

	Collection::Vector<uint8_t> SerializeECSUpdateTransmission();

	virtual void Update() = 0;
};
}
