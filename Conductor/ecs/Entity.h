#pragma once

#include <ecs/ComponentID.h>
#include <ecs/EntityID.h>

#include <collection/Vector.h>

namespace ECS
{
class EntityInfo;

/**
 * An entity is anything in the world.
 */
class Entity final
{
public:
	using Info = EntityInfo;

	explicit Entity(const EntityID& id)
		: m_id(id)
		, m_components()
	{}

	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	Entity(Entity&&) = default;
	Entity& operator=(Entity&&) = default;

	~Entity() {}

	template <typename ComponentType>
	ComponentID FindComponentID() const;

public:
	// A unique ID for this entity.
	EntityID m_id;
	// The components this entity is composed of.
	Collection::Vector<ComponentID> m_components;
	// Explicit padding.
	uint8_t padding[8];
};

template <typename ComponentType>
ComponentID Entity::FindComponentID() const
{
	// TODO This is currently a linear search, which callers will typically follow up with a binary search
	//      to actually get the component. It's worth remembering that this may be a performance concern.
	for (const auto& id : m_components)
	{
		if (id.GetType() == ComponentType::Info::sk_typeHash)
		{
			return id;
		}
	}
	return ActorComponentID();
}
}
