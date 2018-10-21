#pragma once

#include <collection/Vector.h>
#include <ecs/ComponentID.h>

namespace Asset { class AssetManager; }

namespace ECS
{
class ComponentVector;

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

	// Save() must generate a list of bytes which can be used to restore the state of the component using Load(...).
	// Save() and Load(...) don't need to serialize the type of the component.
	virtual void Save(Collection::Vector<uint8_t>& outBytes) const {}
	virtual void Load(const Collection::Vector<uint8_t>& bytes) {}

	ComponentID m_id;
};
}
