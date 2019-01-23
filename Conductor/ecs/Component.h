#pragma once

#include <ecs/ComponentID.h>

namespace Asset { class AssetManager; }

namespace ECS
{
class ComponentVector;

enum class ComponentBindingType
{
	Normal,
	Tag,
	MemoryImaged,
	Networked
};

/**
 * A Component holds data for an entity. Components must define a static const char* k_typeName and
 * a static const Util::StringHash k_typeHash that are unique for their component type.
 * Components must also define a static constexpr ComponentBindingType k_bindingType.
 * Dependening on how they are registered with the ComponentReflector, Components may have additional requirements.
 */
class Component
{
public:
	explicit Component(const ComponentID id)
		: m_id(id)
	{}

	ComponentID m_id;
};
}
