#include <ecs/EntityManager.h>

#include <ecs/Component.h>
#include <ecs/ComponentFactory.h>
#include <ecs/ComponentVector.h>
#include <ecs/ECSGroupVector.h>
#include <ecs/Entity.h>
#include <ecs/EntityInfo.h>
#include <ecs/System.h>

#include <collection/ArrayView.h>

#include <algorithm>
#include <execution>
#include <set>

namespace ECS
{
EntityManager::EntityManager(const ComponentFactory& componentFactory)
	: m_componentFactory(componentFactory)
{
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

void EntityManager::AddECSIndicesToSystems(const Collection::ArrayView<Entity>& entitiesToAdd)
{
	for (auto& executionGroup : m_concurrentSystemGroups)
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

void EntityManager::Update()
{
	UpdateSystems();
}

void EntityManager::UpdateSystems()
{
	const auto RecalculateECSGroupVectors = [this]()
	{
		if (m_ecsGroupVectorsNeedRecalculation)
		{
			for (auto& concurrentGroup : m_concurrentSystemGroups)
			{
				for (auto& registeredSystem : concurrentGroup.m_systems)
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

	// Update the concurrent system groups.
	for (auto& concurrentGroup : m_concurrentSystemGroups)
	{
		// The systems in each group can update in parallel.
		std::for_each(std::execution::par, concurrentGroup.m_systems.begin(), concurrentGroup.m_systems.end(),
			[&](RegisteredSystem& registeredSystem)
		{
			registeredSystem.m_updateFunction(*this, registeredSystem);
		});

		// Resolve deferred functions single-threaded.
		for (auto& registeredSystem : concurrentGroup.m_systems)
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
