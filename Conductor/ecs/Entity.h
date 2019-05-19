#pragma once

#include <ecs/ComponentID.h>
#include <ecs/EntityFlags.h>
#include <ecs/EntityID.h>
#include <ecs/EntityLayer.h>

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
 * Each entity is in exactly one layer. A layer is a set of entities within an EntityManager.
 */
class alignas(64) Entity final
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
	const EntityFlags& GetFlags() const { return m_flags; }
	const EntityLayer& GetLayer() const { return m_layer; }
	const Entity* GetParent() const { return m_parent; }
	Collection::ArrayView<const Entity* const> GetChildren() const { return m_children.GetView(); }
	const Collection::Vector<ComponentID>& GetComponentIDs() const { return m_componentIDs; }

	template <typename TComponent>
	ComponentID FindComponentID() const;

	ComponentID FindComponentID(const ComponentType& componentType) const;

private:
	friend class EntityManager;

	// A unique ID for this entity.
	EntityID m_id;

	EntityFlags m_flags;
	EntityLayer m_layer;
	uint8_t m_padding[13];

	Entity* m_parent{ nullptr };
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
