#pragma once

#include <ecs/ComponentID.h>

namespace Asset { class AssetManager; }

namespace ECS
{
class ComponentVector;

enum class ComponentBindingType
{
	Normal,          // The component implements full serialization and deserialization.
	Tag,             // The component cannot be instantiated, and therefore doesn't serialize or deserialize.
	MemoryImaged,    // The component's serialization is performed automatically using memcpy.
	NetworkedNormal, // The component is network synchronized. It implements full and delta serialization and deserialization.
	NetworkedMemoryImaged // The component is network synchronized. Its serialization is performed automatically using memcpy.
};

/**
 * A Component holds data for an entity. Components must define a static const char* k_typeName and
 * a static const ECS::ComponentType k_type that are unique for their component type.
 * Components must also define a static constexpr ComponentBindingType k_bindingType.
 * Dependening on how they are registered with the ComponentReflector, Components may have additional requirements.
 *
 * NETWORKING NOTE: The presence of components is always synchronized between client and server, even when the component
 *                  data isn't.
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
