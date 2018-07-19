#pragma once

#include <collection/ArrayView.h>
#include <collection/Vector.h>
#include <collection/VectorMap.h>

#include <ecs/ComponentID.h>
#include <ecs/ECSGroupVector.h>
#include <ecs/EntityID.h>

#include <mem/UniquePtr.h>

#include <util/StringHash.h>

#include <functional>
#include <type_traits>

namespace Behave
{
class BehaveContext;
}

namespace Collection
{
template <typename T>
class ArrayView;
}

namespace ECS
{
class Component;
class ComponentFactory;
class ComponentVector;
class ECSGroupVector;
class Entity;
class EntityInfo;
class System;

/**
 * An entity manager owns and updates entities.
 */
class EntityManager final
{
public:
	EntityManager(const ComponentFactory& componentFactory);
	~EntityManager();

	Entity& CreateEntity(const EntityInfo& actorInfo);

	Entity* FindEntity(const EntityID id);
	const Entity* FindEntity(const EntityID id) const;

	Component* FindComponent(const ComponentID id);
	const Component* FindComponent(const ComponentID id) const;

	size_t FindComponentIndex(const ComponentID id) const;

	Entity& GetEntityByIndex(const size_t index);
	Component& GetComponentByIndex(const Util::StringHash typeHash, const size_t index);

	void RemoveComponent(const ComponentID id);

	template <typename BehaviourSystemType>
	void RegisterSystem(Mem::UniquePtr<BehaviourSystemType>&& system);

	void Update(const Behave::BehaveContext& context);

private:
	struct RegisteredSystem;
	using SystemUpdateFn = void(*)(EntityManager&, const Behave::BehaveContext&, RegisteredSystem&);

	struct RegisteredSystem
	{
		RegisteredSystem();
		RegisteredSystem(Mem::UniquePtr<System>&& system, SystemUpdateFn updateFunction);

		Mem::UniquePtr<System> m_system;
		SystemUpdateFn m_updateFunction;
		ECSGroupVector m_ecsGroups;
		Collection::Vector<std::function<void()>> m_deferredFunctions;
	};

	struct SystemExecutionGroup
	{
		Collection::Vector<RegisteredSystem> m_systems;
	};

	void RegisterSystem(Mem::UniquePtr<System>&& system, SystemUpdateFn updateFn);

	void AddECSIndicesToSystems(const Collection::ArrayView<Entity>& entitiesToAdd);

	void UpdateSystems(const Behave::BehaveContext& context);

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
	Collection::Vector<SystemExecutionGroup> m_systemExecutionGroups{};

	// Whether or not the ECS group vectors need to be recalculated.
	bool m_ecsGroupVectorsNeedRecalculation{ false };
};

template <typename SystemType>
void EntityManager::RegisterSystem(Mem::UniquePtr<SystemType>&& system)
{
	struct SystemTypeFunctions
	{
		static void Update(EntityManager& entityManager, const Behave::BehaveContext& context,
			RegisteredSystem& registeredSystem)
		{
			const auto& system = static_cast<const SystemType&>(*registeredSystem.m_system);
			const auto ecsGroupsView =
				registeredSystem.m_ecsGroups.GetView<SystemType::ECSGroupType>();

			system.Update(entityManager, context, ecsGroupsView, registeredSystem.m_deferredFunctions);
		}
	};
	RegisterSystem(std::move(system), &SystemTypeFunctions::Update);
}
}
