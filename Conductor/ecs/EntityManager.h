#pragma once

#include <collection/ArrayView.h>
#include <collection/LinearBlockHashMap.h>
#include <collection/Variant.h>
#include <collection/Vector.h>
#include <collection/VectorMap.h>

#include <ecs/ApplyDeltaTransmissionResult.h>
#include <ecs/ComponentID.h>
#include <ecs/ECSGroupVector.h>
#include <ecs/Entity.h>
#include <ecs/EntityID.h>
#include <ecs/SystemUtil.h>

#include <mem/UniquePtr.h>

#include <unit/Time.h>

#include <functional>
#include <type_traits>

namespace Asset { class AssetManager; }

namespace ECS
{
class Component;
class ComponentReflector;
class ComponentVector;
class ECSGroupVector;
struct SerializedEntitiesAndComponents;
class System;

/**
 * An entity manager owns and updates Entities composed of Components using Systems.
 * Systems run in the order they are registered.
 */
class EntityManager final
{
public:
	EntityManager(Asset::AssetManager& assetManager,
		const ComponentReflector& componentReflector,
		const EntityID firstEntityID,
		const uint64_t firstComponentID);
	~EntityManager();

	Entity& CreateEntityWithComponents(
		const Collection::ArrayView<const ComponentType>& componentTypes,
		const EntityFlags flags,
		const EntityLayer layer,
		const EntityID requestedID = EntityID());

	Collection::Vector<Entity*> CreateEntitiesFromFullSerialization(
		const SerializedEntitiesAndComponents& serialization);
	void SetNetworkedEntitiesToFullSerialization(const SerializedEntitiesAndComponents& serialization);

	void FullySerializeEntitiesAndComponents(
		const Collection::ArrayView<const Entity*>& entities,
		SerializedEntitiesAndComponents& serialization) const;
	void FullySerializeAllEntitiesAndComponentsMatchingFilter(
		const std::function<bool(const Entity&)>& filter,
		SerializedEntitiesAndComponents& serialization) const;

	void SetParentEntity(Entity& entity, Entity* parentEntity);
	void DeleteEntities(const Collection::ArrayView<const EntityID>& entitiesToDelete);

	Entity* FindEntity(const EntityID id);
	const Entity* FindEntity(const EntityID id) const;

	Component* FindComponent(const ComponentID id);
	const Component* FindComponent(const ComponentID id) const;

	template <typename TComponent>
	TComponent* FindComponent(const Entity& entity);
	template <typename TComponent>
	const TComponent* FindComponent(const Entity& entity) const;

	// Register a system to run by itself. Systems run in the order they are registered.
	template <typename SystemType>
	SystemType& RegisterSystem(Mem::UniquePtr<SystemType>&& system);

	// Register multiple systems to run concurrently.
	template <typename... SystemTypes>
	void RegisterConcurrentSystems(Mem::UniquePtr<SystemTypes>&&... concurrentSystems);

	// Run the EntityManager one step.
	void Update(const Unit::Time::Millisecond delta);

private:
	struct RegisteredSystem;
	using SystemUpdateFn = void(*)(RegisteredSystem&, Unit::Time::Millisecond);
	using NotifyOfEntityFn = void(*)(RegisteredSystem&, EntityID, const Collection::Vector<void*>&);

	struct RegisteredSystem
	{
		RegisteredSystem();
		RegisteredSystem(Mem::UniquePtr<System>&& system, SystemUpdateFn updateFunction,
			NotifyOfEntityFn notifyEntityAddedFunction, NotifyOfEntityFn notifyEntityRemovedFunction);

		Mem::UniquePtr<System> m_system;
		SystemUpdateFn m_updateFunction;
		NotifyOfEntityFn m_notifyEntityAddedFunction;
		NotifyOfEntityFn m_notifyEntityRemovedFunction;
		ECSGroupVector m_ecsGroups;
		Collection::Vector<std::function<void(EntityManager&)>> m_deferredFunctions;
	};

	struct RegisteredConcurrentSystemGroup
	{
		Collection::Vector<RegisteredSystem> m_systems;
	};

	template <typename SystemType> struct SystemTypeFunctions;

	// Access a ComponentVector, initializing it if necessary. Returns null for tag components.
	ComponentVector* GetComponentVector(const ComponentType componentType);
	
	// Add a component to an entity.
	void AddComponentToEntity(const ComponentType componentType, Entity& entity);
	void AddComponentToEntity(
		const ComponentType componentType, const uint8_t*& bytes, const uint8_t* bytesEnd, Entity& entity);
	// Remove a component from this EntityManager. Does not remove it from the entity referencing it.
	void RemoveComponent(const ComponentID id);

	template <typename SystemType>
	SystemType& RegisterSystemInGroup(Mem::UniquePtr<SystemType>&& system, RegisteredConcurrentSystemGroup& outGroup);

	void AddECSPointersToSystems(Entity& entityToAdd);
	void AddECSPointersToSystems(Collection::ArrayView<Entity* const> entitiesToAdd);
	void RemoveECSPointersFromSystems(Entity& entity);
	
private:
	// Components that load resources from disk use the AssetManager to do so efficiently.
	Asset::AssetManager& m_assetManager;

	// A reflected database of functions for manipulating components.
	const ComponentReflector& m_componentReflector;

	// The entities this manager is in charge of updating.
	class EntityIDHashFunctor final : Collection::I64HashFunctor
	{
	public:
		uint64_t Hash(const EntityID& key) const { return I64HashFunctor::Hash(static_cast<int64_t>(key.GetUniqueID())); }
	};
	Collection::LinearBlockHashMap<EntityID, Entity, EntityIDHashFunctor> m_entities;

	// The components this manager owns on behalf of its entities, grouped by type.
	Collection::VectorMap<ComponentType, ComponentVector> m_components{};

	// The next entity ID that will be assigned.
	EntityID m_nextEntityID{ 0 };

	// The next component ID that will be assigned.
	uint64_t m_nextComponentID{ 0 };

	// The systems that this entity manager is running, sorted into groups which can run concurrently.
	Collection::Vector<RegisteredConcurrentSystemGroup> m_concurrentSystemGroups{};
};

template <typename TComponent>
inline TComponent* EntityManager::FindComponent(const Entity& entity)
{
	const ComponentID componentID = entity.FindComponentID<TComponent>();
	Component* const component = FindComponent(componentID);
	if (component != nullptr)
	{
		return static_cast<TComponent*>(component);
	}
	return nullptr;
}

template <typename TComponent>
inline const TComponent* EntityManager::FindComponent(const Entity& entity) const
{
	const ComponentID componentID = entity.FindComponentID<TComponent>();
	const Component* const component = FindComponent(componentID);
	if (component != nullptr)
	{
		return static_cast<const TComponent*>(component);
	}
	return nullptr;
}

template <typename SystemType>
inline SystemType& EntityManager::RegisterSystem(Mem::UniquePtr<SystemType>&& system)
{
	AMP_FATAL_ASSERT(m_entities.IsEmpty(), "Systems must be registered before entities are added to the "
		"EntityManager because there is not currently support for initializing the system's component groups.");

	RegisteredConcurrentSystemGroup& newGroup = m_concurrentSystemGroups.Emplace();
	return RegisterSystemInGroup<SystemType>(std::move(system), newGroup);
}

template <typename... SystemTypes>
inline void EntityManager::RegisterConcurrentSystems(Mem::UniquePtr<SystemTypes>&&... concurrentSystems)
{
	static_assert(SystemUtil::AreSystemsWriteCompatible<SystemTypes...>(),
		"The given systems can't run concurrently due to write conflicts.");

	AMP_FATAL_ASSERT(m_entities.IsEmpty(), "Systems must be registered before entities are added to the "
		"EntityManager because there is not currently support for initializing the system's component groups.");

	RegisteredConcurrentSystemGroup& newGroup = m_concurrentSystemGroups.Emplace();
	(... , RegisterSystemInGroup<SystemTypes>(std::move(concurrentSystems), newGroup));
}

template <typename SystemType>
struct EntityManager::SystemTypeFunctions
{
	static void Update(EntityManager::RegisteredSystem& registeredSystem, const Unit::Time::Millisecond delta)
	{
		SystemType& system = static_cast<SystemType&>(*registeredSystem.m_system);
		const auto ecsGroupsView =
			registeredSystem.m_ecsGroups.GetView<SystemType::ECSGroupType>();

		system.Update(delta, ecsGroupsView, registeredSystem.m_deferredFunctions);
	}

	static void NotifyEntityAdded(EntityManager::RegisteredSystem& registeredSystem,
		const EntityID id, const Collection::Vector<void*>& rawGroup)
	{
		if constexpr (SystemType::k_bindingType == SystemBindingType::Extended)
		{
			SystemType& system = static_cast<SystemType&>(*registeredSystem.m_system);
			const SystemType::ECSGroupType& group = *reinterpret_cast<const SystemType::ECSGroupType*>(&rawGroup[0]);
			system.NotifyOfEntityAdded(id, group);
		}
	}

	static void NotifyEntityRemoved(EntityManager::RegisteredSystem& registeredSystem,
		const EntityID id, const Collection::Vector<void*>& rawGroup)
	{
		if constexpr (SystemType::k_bindingType == SystemBindingType::Extended)
		{
			SystemType& system = static_cast<SystemType&>(*registeredSystem.m_system);
			const SystemType::ECSGroupType& group = *reinterpret_cast<const SystemType::ECSGroupType*>(&rawGroup[0]);
			system.NotifyOfEntityRemoved(id, group);
		}
	}
};

template <typename SystemType>
inline SystemType& EntityManager::RegisterSystemInGroup(Mem::UniquePtr<SystemType>&& system,
	RegisteredConcurrentSystemGroup& outGroup)
{
	RegisteredSystem& registeredSystem = outGroup.m_systems.Emplace(std::move(system),
		&SystemTypeFunctions<SystemType>::Update,
		&SystemTypeFunctions<SystemType>::NotifyEntityAdded,
		&SystemTypeFunctions<SystemType>::NotifyEntityRemoved);
	return *static_cast<SystemType*>(registeredSystem.m_system.Get());
}
}
