#pragma once

#include <ecs/ComponentID.h>
#include <ecs/EntityID.h>

#include <collection/Vector.h>
#include <util/StringHash.h>

namespace ECS
{
class EntityManager;

/**
 * An entity is anything in the world.
 * Entities are composed of components. An entity's components are determined by its EntityInfo.
 * Entities may have a parent. An entity is destroyed if its parent is destroyed. Certain components and systems take
 * an entity's parent into account when updating.
 */
class Entity final
{
public:
	static constexpr const char* const k_typeName = "entity";
	static const ECS::ComponentType k_type;

	Entity() = default;

	explicit Entity(const EntityID& id)
		: m_id(id)
		, m_componentIDs()
	{}

	Entity(const Entity&) = delete;
	Entity& operator=(const Entity&) = delete;

	Entity(Entity&&) = default;
	Entity& operator=(Entity&&) = default;

	~Entity() {}

	const EntityID& GetID() const { return m_id; }
	const Entity* GetParent() const { return m_parent; }
	Collection::ArrayView<const Entity* const> GetChildren() const { return m_children.GetView(); }
	const Collection::Vector<ComponentID>& GetComponentIDs() const { return m_componentIDs; }

	template <typename TComponent>
	ComponentID FindComponentID() const;

	ComponentID FindComponentID(const ComponentType& componentType) const;

private:
	friend class EntityManager;

	uint8_t m_padding[18];

	// A unique ID for this entity.
	EntityID m_id;
	
	// The parent of this entity.
	Entity* m_parent{ nullptr };
	// The children of this entity.
	Collection::Vector<Entity*> m_children;

	// The components this entity is composed of.
	Collection::Vector<ComponentID> m_componentIDs;
};

template <typename TComponent>
inline ComponentID Entity::FindComponentID() const
{
	return FindComponentID(TComponent::k_type);
}

inline ComponentID Entity::FindComponentID(const ComponentType& componentType) const
{
	// TODO(ecs) This is currently a linear search, which callers will typically follow up with a binary search
	//      to actually get the component. It's worth remembering that this may be a performance concern.
	for (const auto& id : m_componentIDs)
	{
		if (id.GetType() == componentType)
		{
			return id;
		}
	}
	return ComponentID();
}
}
