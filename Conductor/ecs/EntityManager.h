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

#include <functional>
#include <type_traits>

namespace Asset { class AssetManager; }

namespace ECS
{
class Component;
class ComponentInfo;
class ComponentReflector;
class ComponentVector;
class ECSGroupVector;
class EntityInfo;
class System;

/**
 * An entity manager owns and updates Entities composed of Components using Systems.
 * Systems run in the order they are registered.
 */
class EntityManager final
{
public:
	EntityManager(Asset::AssetManager& assetManager, const ComponentReflector& componentReflector, bool transmitsState);
	~EntityManager();

	Entity& CreateEntity(const EntityInfo& entityInfo);
	void SetInfoForEntity(const EntityInfo& entityInfo, Entity& entity);
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
	void RegisterSystem(Mem::UniquePtr<SystemType>&& system);

	// Register multiple systems to run concurrently.
	template <typename... SystemTypes>
	void RegisterConcurrentSystems(Mem::UniquePtr<SystemTypes>&&... concurrentSystems);

	// Run the EntityManager one step.
	void Update();

	// Calculate a delta update transmission for the entitites in the manager.
	// This is a delta since the last delta package was requested.
	Collection::Vector<uint8_t> SerializeDeltaTransmission();

	// Apply a delta update transmission to the entities in the manager.
	ApplyDeltaTransmissionResult ApplyDeltaTransmission(const Collection::Vector<uint8_t>& transmissionBytes);

private:
	struct RegisteredSystem;
	using SystemUpdateFn = void(*)(RegisteredSystem&);
	using NotifyOfEntityFn = void(*)(RegisteredSystem&, const EntityID&, const Collection::Vector<void*>&);

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
	
	// Add a component to an entity.
	void AddComponentToEntity(const ComponentInfo& componentInfo, Entity& entity);
	// Remove a component from this EntityManager. Does not remove it from the entity referencing it.
	void RemoveComponent(const ComponentID id);

	template <typename SystemType>
	void RegisterSystemInGroup(Mem::UniquePtr<SystemType>&& system, RegisteredConcurrentSystemGroup& outGroup);

	void AddECSPointersToSystems(Collection::ArrayView<Entity>& entitiesToAdd);
	void RemoveECSPointersFromSystems(Entity& entity);
	
	void UpdateSystems();
	
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

	// The buffered state this manager stores in order to calculate delta updates for transmission.
	// m_transmissionBuffers is null for managers that do not transmit state.
	struct TransmissionBuffers
	{
		// Only components that are network-enabled have a buffered copy stored.
		Collection::VectorMap<ComponentType, ComponentVector> m_bufferedComponents;

		Collection::Vector<EntityID> m_entitiesAddedSinceLastTransmission;
		Collection::Vector<EntityID> m_entitiesChangedSinceLastTransmission;
		Collection::Vector<EntityID> m_entitiesRemovedSinceLastTransmission;
		Collection::Vector<ComponentID> m_componentsAddedSinceLastTransmission;
		Collection::Vector<ComponentID> m_componentsRemovedSinceLastTransmission;
	};
	Mem::UniquePtr<TransmissionBuffers> m_transmissionBuffers;

	// The next entity ID that will be assigned.
	EntityID m_nextEntityID{ 0 };

	// The next component ID that will be assigned.
	size_t m_nextComponentID{ 0 };

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
inline void EntityManager::RegisterSystem(Mem::UniquePtr<SystemType>&& system)
{
	Dev::FatalAssert(m_entities.IsEmpty(), "Systems must be registered before entities are added to the "
		"EntityManager because there is not currently support for initializing the system's component groups.");

	RegisteredConcurrentSystemGroup& newGroup = m_concurrentSystemGroups.Emplace();
	RegisterSystemInGroup<SystemType>(std::move(system), newGroup);
}

template <typename... SystemTypes>
inline void EntityManager::RegisterConcurrentSystems(Mem::UniquePtr<SystemTypes>&&... concurrentSystems)
{
	static_assert(SystemUtil::AreSystemsWriteCompatible<SystemTypes...>(),
		"The given systems can't run concurrently due to write conflicts.");

	Dev::FatalAssert(m_entities.IsEmpty(), "Systems must be registered before entities are added to the "
		"EntityManager because there is not currently support for initializing the system's component groups.");

	RegisteredConcurrentSystemGroup& newGroup = m_concurrentSystemGroups.Emplace();
	(... , RegisterSystemInGroup<SystemTypes>(std::move(concurrentSystems), newGroup));
}

template <typename SystemType>
struct EntityManager::SystemTypeFunctions
{
	static void Update(EntityManager::RegisteredSystem& registeredSystem)
	{
		SystemType& system = static_cast<SystemType&>(*registeredSystem.m_system);
		const auto ecsGroupsView =
			registeredSystem.m_ecsGroups.GetView<SystemType::ECSGroupType>();

		system.Update(ecsGroupsView, registeredSystem.m_deferredFunctions);
	}

	static void NotifyEntityAdded(EntityManager::RegisteredSystem& registeredSystem,
		const EntityID& id, const Collection::Vector<void*>& rawGroup)
	{
		if constexpr (SystemType::k_bindingType == SystemBindingType::Extended)
		{
			SystemType& system = static_cast<SystemType&>(*registeredSystem.m_system);
			const SystemType::ECSGroupType& group = *reinterpret_cast<const SystemType::ECSGroupType*>(&rawGroup[0]);
			system.NotifyOfEntityAdded(id, group);
		}
	}

	static void NotifyEntityRemoved(EntityManager::RegisteredSystem& registeredSystem,
		const EntityID& id, const Collection::Vector<void*>& rawGroup)
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
inline void EntityManager::RegisterSystemInGroup(Mem::UniquePtr<SystemType>&& system,
	RegisteredConcurrentSystemGroup& outGroup)
{
	outGroup.m_systems.Emplace(std::move(system),
		&SystemTypeFunctions<SystemType>::Update,
		&SystemTypeFunctions<SystemType>::NotifyEntityAdded,
		&SystemTypeFunctions<SystemType>::NotifyEntityRemoved);
}
}
