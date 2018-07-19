#include <ecs/EntityManager.h>

#include <behave/BehaveContext.h>
#include <ecs/Component.h>
#include <ecs/ComponentFactory.h>
#include <ecs/ComponentVector.h>
#include <ecs/ECSGroupVector.h>
#include <ecs/Entity.h>
#include <ecs/EntityInfo.h>
#include <ecs/System.h>
#include <ecs/systems/BehaviourTreeEvaluationSystem.h>

#include <collection/ArrayView.h>

#include <algorithm>
#include <execution>
#include <set>

namespace ECS
{
EntityManager::EntityManager(const ComponentFactory& componentFactory)
	: m_componentFactory(componentFactory)
{
	RegisterSystem(Mem::MakeUnique<Systems::BehaviourTreeEvaluationSystem>());
}

EntityManager::~EntityManager()
{
}

Entity& EntityManager::CreateEntity(const EntityInfo& entityInfo)
{
	// Create the enttiy.
	const EntityID entityID = m_nextEntityID;
	Entity& entity = m_entities.Emplace(entityID);

	// Create the entity's components using our component factory.
	for (const auto& componentInfo : entityInfo.m_componentInfos)
	{
		const Util::StringHash componentTypeHash = componentInfo->GetTypeHash();
		const ComponentID componentID{ componentTypeHash, m_nextComponentID };

		ComponentVector& componentVector = m_components[componentTypeHash];
		if (componentVector.GetComponentType() == Util::StringHash())
		{
			// This is a type that has not yet been encountered and therefore must be initialized.
			componentVector = ComponentVector(componentTypeHash,
				m_componentFactory.GetSizeOfComponentInBytes(componentTypeHash));
		}
		Dev::FatalAssert(componentVector.GetComponentType() == componentTypeHash,
			"Mismatch between component vector type and the key it is stored at.");

		if (!m_componentFactory.TryMakeComponent(*componentInfo, componentID, componentVector))
		{
			Dev::LogWarning("Failed to create component of type [%s].", componentInfo->GetTypeName());
			continue;
		}

		++m_nextComponentID;
		entity.m_components.Add(componentID);
	}
	
	// Update the next unique ID.
	m_nextEntityID = EntityID(entityID.GetUniqueID() + 1);

	// Add the entity's components to the system execution groups.
	AddECSIndicesToSystems(Collection::ArrayView<Entity>(&entity, 1));

	// Return the entity after it is fully initialized.
	return entity;
}

Entity* EntityManager::FindEntity(const EntityID id)
{
	// Implemented using the const variant.
	return const_cast<Entity*>(static_cast<const EntityManager*>(this)->FindEntity(id));
}

const Entity* EntityManager::FindEntity(const EntityID id) const
{
	const auto itr = std::lower_bound(m_entities.begin(), m_entities.end(), id,
		[](const Entity& entity, const EntityID& id)
	{
		return entity.m_id < id;
	});
	if (itr == m_entities.end() || itr->m_id != id)
	{
		return nullptr;
	}
	return &*itr;
}

Component* EntityManager::FindComponent(const ComponentID id)
{
	// Implemented using the const variant.
	return const_cast<Component*>(static_cast<const EntityManager*>(this)->FindComponent(id));
}

const Component* EntityManager::FindComponent(const ComponentID id) const
{
	const Collection::Pair<const Util::StringHash, ComponentVector>* const componentsEntry =
		m_components.Find(id.GetType());
	if (componentsEntry == nullptr)
	{
		return nullptr;
	}
	const ComponentVector& components = componentsEntry->second;

	const auto itr = std::lower_bound(components.begin(), components.end(), id,
		[](const Component& component, const ComponentID& id)
	{
		return component.m_id < id;
	});
	if (itr == components.end() || itr->m_id != id)
	{
		return nullptr;
	}
	return &*itr;
}

size_t EntityManager::FindComponentIndex(const ComponentID id) const
{
	const Collection::Pair<const Util::StringHash, ComponentVector>* const componentsEntry =
		m_components.Find(id.GetType());
	if (componentsEntry == nullptr)
	{
		return static_cast<size_t>(-1i64);
	}
	const ComponentVector& components = componentsEntry->second;

	const auto itr = std::lower_bound(components.begin(), components.end(), id,
		[](const Component& component, const ComponentID& id)
	{
		return component.m_id < id;
	});
	if (itr == components.end() || itr->m_id != id)
	{
		return static_cast<size_t>(-1i64);
	}
	return static_cast<size_t>(itr.GetIndex());
}

Entity& EntityManager::GetEntityByIndex(const size_t index)
{
	return m_entities[index];
}

Component& EntityManager::GetComponentByIndex(const Util::StringHash typeHash, const size_t index)
{
	return m_components.Find(typeHash)->second[index];
}

void EntityManager::RemoveComponent(const ComponentID id)
{
	// Delete the component.
	Collection::Pair<const Util::StringHash, ComponentVector>* const componentsEntry =
		m_components.Find(id.GetType());
	ComponentVector& components = componentsEntry->second;
	components.Remove(id, m_componentFactory);

	// Flag the ECS group vectors for recalculation.
	m_ecsGroupVectorsNeedRecalculation = true;
}

EntityManager::RegisteredSystem::RegisteredSystem()
	: m_system()
	, m_updateFunction(nullptr)
	, m_ecsGroups()
	, m_deferredFunctions()
{}

EntityManager::RegisteredSystem::RegisteredSystem(
	Mem::UniquePtr<System>&& system,
	SystemUpdateFn updateFunction)
	: m_system(std::move(system))
	, m_updateFunction(updateFunction)
	, m_ecsGroups(m_system->GetImmutableTypes().Size() + m_system->GetMutableTypes().Size())
{}

void EntityManager::RegisterSystem(
	Mem::UniquePtr<System>&& system,
	SystemUpdateFn updateFn)
{
	Dev::FatalAssert(m_entities.IsEmpty(), "Systems must be registered before entities are added to the "
		"EntityManager because there is not currently support for initializing the system's component groups.");

	// Try to find an execution group for which this system does not mutate instances of the referenced types of any system
	// in the group and for which none of the systems in the group mutate instances of the referenced types of this system.
	// Systems which read or write Entities and not just Components are always placed in their own execution group.
	const auto TypeIsEntity = [](const Util::StringHash& type)
	{
		return type == EntityInfo::sk_typeHash;
	};
	const bool systemReferencesEntities =
		std::any_of(system->GetMutableTypes().begin(), system->GetMutableTypes().end(), TypeIsEntity)
		|| std::any_of(system->GetImmutableTypes().begin(), system->GetImmutableTypes().end(), TypeIsEntity);
	
	if (!systemReferencesEntities)
	{
		for (auto& executionGroup : m_systemExecutionGroups)
		{
			std::set<Util::StringHash> groupTotalTypes;
			std::set<Util::StringHash> groupMutableTypes;
			for (const auto& registeredSystem : executionGroup.m_systems)
			{
				for (const auto& type : registeredSystem.m_system->GetImmutableTypes())
				{
					groupTotalTypes.insert(type);
				}
				for (const auto& type : registeredSystem.m_system->GetMutableTypes())
				{
					groupTotalTypes.insert(type);
					groupMutableTypes.insert(type);
				}
			}

			const bool systemMutatesGroup = std::any_of(system->GetMutableTypes().begin(),
				system->GetMutableTypes().end(),
				[&](const Util::StringHash& type)
			{
				return (groupTotalTypes.find(type) != groupTotalTypes.end());
			});
			if (systemMutatesGroup)
			{
				continue;
			}

			const bool groupMutatesSystem = std::any_of(groupMutableTypes.begin(), groupMutableTypes.end(),
				[&](const Util::StringHash& type)
			{
				return (std::find(system->GetImmutableTypes().begin(), system->GetImmutableTypes().end(), type)
					!= system->GetImmutableTypes().end())
					|| (std::find(system->GetMutableTypes().begin(), system->GetMutableTypes().end(), type)
						!= system->GetMutableTypes().end());
			});
			if (groupMutatesSystem)
			{
				continue;
			}

			// The system does not conflict with any other systems in the group, so add it and return.
			executionGroup.m_systems.Emplace(std::move(system), updateFn);
			return;
		}
	}

	// If we fail to find an execution group for the system, create a new one for it.
	SystemExecutionGroup& newGroup = m_systemExecutionGroups.Emplace();
	newGroup.m_systems.Emplace(std::move(system), updateFn);
}

void EntityManager::AddECSIndicesToSystems(const Collection::ArrayView<Entity>& entitiesToAdd)
{
	for (auto& executionGroup : m_systemExecutionGroups)
	{
		// Gather the entity indices and component indices each member of the execution group needs.
		for (auto& registeredSystem : executionGroup.m_systems)
		{
			const Collection::Vector<Util::StringHash>& immutableTypes =
				registeredSystem.m_system->GetImmutableTypes();
			const Collection::Vector<Util::StringHash>& mutableTypes =
				registeredSystem.m_system->GetMutableTypes();

			for (const auto& entity : entitiesToAdd)
			{
				const auto TryGatherIndices = [this](const Collection::Vector<Util::StringHash>& componentTypes,
					const Entity& entity, Collection::Vector<size_t>& indices)
				{
					bool foundAll = true;
					for (const auto& typeHash : componentTypes)
					{
						if (typeHash == EntityInfo::sk_typeHash)
						{
							indices.Add((&entity) - m_entities.begin());
							continue;
						}

						const ComponentID* const idPtr = entity.m_components.Find(
							[&](const ComponentID& componentID) { return componentID.GetType() == typeHash; });
						if (idPtr == nullptr)
						{
							foundAll = false;
							break;
						}
						indices.Add(FindComponentIndex(*idPtr));
					}
					return foundAll;
				};

				Collection::Vector<size_t> indices;
				if (!TryGatherIndices(immutableTypes, entity, indices))
				{
					continue;
				}
				if (!TryGatherIndices(mutableTypes, entity, indices))
				{
					continue;
				}

				registeredSystem.m_ecsGroups.Add(indices);
			}

			// Sort the group vector in order to make the memory accesses as fast as possible.
			registeredSystem.m_ecsGroups.Sort();
		}
	}
}

void EntityManager::Update(const Behave::BehaveContext& context)
{
	UpdateSystems(context);
}

void EntityManager::UpdateSystems(const Behave::BehaveContext& context)
{
	const auto RecalculateECSGroupVectors = [this]()
	{
		if (m_ecsGroupVectorsNeedRecalculation)
		{
			for (auto& executionGroup : m_systemExecutionGroups)
			{
				for (auto& registeredSystem : executionGroup.m_systems)
				{
					registeredSystem.m_ecsGroups.Clear();
				}
			}
			AddECSIndicesToSystems({ &m_entities[0], m_entities.Size() });
			
			m_ecsGroupVectorsNeedRecalculation = false;
		}
	};

	// Recalculate ECS group vectors before updating the systems to ensure they have the latest data.
	RecalculateECSGroupVectors();

	// Update the system execution groups.
	for (auto& executionGroup : m_systemExecutionGroups)
	{
		// The systems in each group can update in parallel.
		std::for_each(std::execution::par, executionGroup.m_systems.begin(), executionGroup.m_systems.end(),
			[&](RegisteredSystem& registeredSystem)
		{
			registeredSystem.m_updateFunction(*this, context, registeredSystem);
		});

		// Resolve deferred functions single-threaded.
		for (auto& registeredSystem : executionGroup.m_systems)
		{
			for (auto& deferredFunction : registeredSystem.m_deferredFunctions)
			{
				deferredFunction();
			}
			registeredSystem.m_deferredFunctions.Clear();
		}

		// Recalculate all ECS groups after the deferred functions evaluate.
		RecalculateECSGroupVectors();
	}
}
}
