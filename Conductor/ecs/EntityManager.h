#pragma once

#include <collection/ArrayView.h>
#include <collection/Vector.h>
#include <collection/VectorMap.h>

#include <ecs/ComponentID.h>
#include <ecs/ECSGroupVector.h>
#include <ecs/EntityID.h>
#include <ecs/SystemUtil.h>

#include <mem/UniquePtr.h>

#include <util/StringHash.h>

#include <functional>
#include <type_traits>

namespace Collection
{
template <typename T>
class ArrayView;
}

namespace ECS
{
class Component;
class ComponentFactory;
class ComponentInfo;
class ComponentVector;
class ECSGroupVector;
class Entity;
class EntityInfo;
class System;

/**
 * An entity manager owns and updates Entities composed of Components using Systems.
 * Systems run in the order they are registered.
 */
class EntityManager final
{
public:
	explicit EntityManager(const ComponentFactory& componentFactory);
	~EntityManager();

	Entity& CreateEntity(const EntityInfo& entityInfo);
	void SetInfoForEntity(const EntityInfo& entityInfo, Entity& entity);
	void DeleteEntities(const Collection::ArrayView<const Entity* const>& entitiesToDelete);

	Entity* FindEntity(const EntityID id);
	const Entity* FindEntity(const EntityID id) const;

	Component* FindComponent(const ComponentID id);
	const Component* FindComponent(const ComponentID id) const;

	size_t FindComponentIndex(const ComponentID id) const;

	Entity& GetEntityByIndex(const size_t index);
	Component& GetComponentByIndex(const Util::StringHash typeHash, const size_t index);

	// Register a system to run by itself. Systems run in the order they are registered.
	template <typename SystemType>
	void RegisterSystem(Mem::UniquePtr<SystemType>&& system);

	// Register multiple systems to run concurrently.
	template <typename... SystemTypes>
	void RegisterConcurrentSystems(Mem::UniquePtr<SystemTypes>&&... concurrentSystems);

	void Update();

private:
	struct RegisteredSystem;
	using SystemUpdateFn = void(*)(EntityManager&, RegisteredSystem&);

	struct RegisteredSystem
	{
		RegisteredSystem();
		RegisteredSystem(Mem::UniquePtr<System>&& system, SystemUpdateFn updateFunction);

		Mem::UniquePtr<System> m_system;
		SystemUpdateFn m_updateFunction;
		ECSGroupVector m_ecsGroups;
		Collection::Vector<std::function<void()>> m_deferredFunctions;
	};

	struct RegisteredConcurrentSystemGroup
	{
		Collection::Vector<RegisteredSystem> m_systems;
	};

	// Add a component to an entity.
	void AddComponentToEntity(const ComponentInfo& componentInfo, Entity& entity);
	// Remove a component from this EntityManager. Does not remove it from the entity references it.
	void RemoveComponent(const ComponentID id);

	template <typename SystemType>
	void RegisterSystemInGroup(Mem::UniquePtr<SystemType>&& system, RegisteredConcurrentSystemGroup& outGroup);

	void AddECSIndicesToSystems(const Collection::ArrayView<Entity>& entitiesToAdd);

	void UpdateSystems();

	// The factory this manager uses to create components.
	const ComponentFactory& m_componentFactory;

	// The entities this manager is in charge of updating, sorted by ID.
	Collection::Vector<Entity> m_entities{};

	// The components this manager owns on behalf of its entities, grouped by type.
	Collection::VectorMap<Util::StringHash, ComponentVector> m_components{};

	// The next entity ID that will be assigned.
	EntityID m_nextEntityID{ 0 };

	// The next component ID that will be assigned.
	size_t m_nextComponentID{ 0 };

	// The systems that this entity manager is running, sorted into groups which can run concurrently.
	Collection::Vector<RegisteredConcurrentSystemGroup> m_concurrentSystemGroups{};

	// Whether or not the ECS group vectors need to be recalculated.
	bool m_ecsGroupVectorsNeedRecalculation{ false };
};

template <typename SystemType>
void EntityManager::RegisterSystem(Mem::UniquePtr<SystemType>&& system)
{
	Dev::FatalAssert(m_entities.IsEmpty(), "Systems must be registered before entities are added to the "
		"EntityManager because there is not currently support for initializing the system's component groups.");

	RegisteredConcurrentSystemGroup& newGroup = m_concurrentSystemGroups.Emplace();
	RegisterSystemInGroup<SystemType>(std::move(system), newGroup);
}

template <typename... SystemTypes>
void EntityManager::RegisterConcurrentSystems(Mem::UniquePtr<SystemTypes>&&... concurrentSystems)
{
	static_assert(SystemUtil::AreSystemsWriteCompatible<SystemTypes...>(),
		"The given systems can't run concurrently due to write conflicts.");

	Dev::FatalAssert(m_entities.IsEmpty(), "Systems must be registered before entities are added to the "
		"EntityManager because there is not currently support for initializing the system's component groups.");

	RegisteredConcurrentSystemGroup& newGroup = m_concurrentSystemGroups.Emplace();
	(... , RegisterSystemInGroup<SystemTypes>(std::move(concurrentSystems), newGroup));
}

template <typename SystemType>
void EntityManager::RegisterSystemInGroup(Mem::UniquePtr<SystemType>&& system,
	RegisteredConcurrentSystemGroup& outGroup)
{
	struct SystemTypeFunctions
	{
		static void Update(EntityManager& entityManager, RegisteredSystem& registeredSystem)
		{
			const auto& system = static_cast<const SystemType&>(*registeredSystem.m_system);
			const auto ecsGroupsView =
				registeredSystem.m_ecsGroups.GetView<SystemType::ECSGroupType>();

			system.Update(entityManager, ecsGroupsView, registeredSystem.m_deferredFunctions);
		}
	};
	outGroup.m_systems.Emplace(std::move(system), &SystemTypeFunctions::Update);
}
}
