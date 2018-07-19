#pragma once

#include <ecs/ComponentID.h>

namespace ECS
{
/**
 * A Component holds data for an entity. A component is always instantiated using a ComponentInfo.
 * Components must define an Info type which they are instantiated from, and must define a TryCreateFromInfo
 * static function which creates them from their info.
 */
class Component
{
public:
	explicit Component(const ComponentID id)
		: m_id(id)
	{}

	virtual ~Component() {}

	ComponentID m_id;
};
}
