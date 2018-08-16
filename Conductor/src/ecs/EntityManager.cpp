#include <ecs/EntityManager.h>

#include <ecs/Component.h>
#include <ecs/ComponentFactory.h>
#include <ecs/ComponentVector.h>
#include <ecs/ECSGroupVector.h>
#include <ecs/Entity.h>
#include <ecs/EntityInfo.h>
#include <ecs/System.h>

#include <collection/ArrayView.h>
#include <mem/Serialize.h>

#include <algorithm>
#include <execution>
#include <set>

namespace ECS
{
namespace Internal_EntityManager
{
enum class ComponentTransmissionType : uint8_t
{
	Added = 0,
	Removed,
	DeltaUpdate
};
}

EntityManager::EntityManager(const ComponentFactory& componentFactory)
	: m_componentFactory(componentFactory)
{
}

EntityManager::~EntityManager()
{
}

Entity& EntityManager::CreateEntity(const EntityInfo& entityInfo)
{
	// Create the entity.
	const EntityID entityID = m_nextEntityID;
	Entity& entity = m_entities.Emplace(entityID, entityInfo.m_nameHash);

	// Create the entity's components using our component factory.
	for (const auto& componentInfo : entityInfo.m_componentInfos)
	{
		AddComponentToEntity(*componentInfo, entity);
	}
	
	// Update the next unique ID.
	m_nextEntityID = EntityID(entityID.GetUniqueID() + 1);

	// Add the entity's components to the system execution groups.
	AddECSIndicesToSystems(Collection::ArrayView<Entity>(&entity, 1));

	// Return the entity after it is fully initialized.
	return entity;
}

void EntityManager::SetInfoForEntity(const EntityInfo& entityInfo, Entity& entity)
{
	// Early out if the entity is already using the given info.
	if (entity.GetInfoNameHash() == entityInfo.m_nameHash)
	{
		return;
	}

	// Remove any components the entity no longer needs.
	for (size_t i = 0; i < entity.m_components.Size();)
	{
		const ComponentID& componentID = entity.m_components[i];

		const auto* const matchingComponentInfo = entityInfo.m_componentInfos.Find(
			[&](const Mem::UniquePtr<ComponentInfo>& componentInfo)
		{
			return componentInfo->GetTypeHash() == componentID.GetType();
		});

		if (matchingComponentInfo == nullptr)
		{
			RemoveComponent(componentID);
			entity.m_components.SwapWithAndRemoveLast(i);
		}
		else
		{
			++i;
		}
	}

	// Add any components the entity is missing.
	for (const auto& componentInfo : entityInfo.m_componentInfos)
	{
		const ComponentID componentID = entity.FindComponentID(componentInfo->GetTypeHash());
		if (componentID != ComponentID())
		{
			continue;
		}
		AddComponentToEntity(*componentInfo, entity);
	}
}

void EntityManager::DeleteEntities(const Collection::ArrayView<const Entity* const>& entitiesToDelete)
{
	const size_t removeIndex = m_entities.Partition([&](const Entity& entity)
	{
		for (const auto& entityToDelete : entitiesToDelete)
		{
			if (entityToDelete == &entity)
			{
				return false;
			}
		}
		return true;
	});
	for (size_t i = removeIndex, iEnd = m_entities.Size(); i < iEnd; ++i)
	{
		const Entity& entity = m_entities[i];
		for (const auto& componentID : entity.GetComponentIDs())
		{
			RemoveComponent(componentID);
		}
	}
	m_entities.Remove(removeIndex, m_entities.Size());
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

Collection::Vector<uint8_t> EntityManager::SerializeDeltaTransmission()
{
	using namespace Internal_EntityManager;

	Collection::Vector<uint8_t> transmissionBytes;

	for (auto& entry : m_bufferedComponents)
	{
		// Serialize the component type string to start the delta update for this component type.
		Mem::Serialize(Util::ReverseHash(entry.first), transmissionBytes);

		const auto serializer = m_componentFactory.FindTransmissionFunctions(entry.first);

		ComponentVector& bufferedComponents = entry.second;
		const ComponentVector& currentComponents = m_components.Find(entry.first)->second;

		auto bufferedIter = bufferedComponents.begin();
		auto currentIter = currentComponents.begin();
		const auto bufferedEnd = bufferedComponents.end();
		const auto currentEnd = currentComponents.end();

		// Each vector of components is sorted by ID. Because of this, they can be iterated over at the same time.
		// Components are each serialized first with their ID and then with a ComponentTransmissionType.
		while (true)
		{
			while (bufferedIter->m_id < currentIter->m_id && (bufferedIter + 1) < bufferedEnd)
			{
				// If the buffered component ID is less than the current component ID,
				// the buffered component was deleted.
				Mem::Serialize(bufferedIter->m_id.GetUniqueID(), transmissionBytes);
				Mem::Serialize(static_cast<uint8_t>(ComponentTransmissionType::Removed), transmissionBytes);

				++bufferedIter;
			}

			if (bufferedIter->m_id == currentIter->m_id)
			{
				// If the IDs are equal, a delta update can be made.
				Mem::Serialize(bufferedIter->m_id.GetUniqueID(), transmissionBytes);
				Mem::Serialize(static_cast<uint8_t>(ComponentTransmissionType::DeltaUpdate), transmissionBytes);
				serializer.m_serializeDeltaTransmissionFunction(*bufferedIter, *currentIter, transmissionBytes);

				++bufferedIter;
				++currentIter;
			}
			else
			{
				// If the buffered component ID is greater than the current component ID,
				// the current component was created and it must be fully serialized.
				Mem::Serialize(currentIter->m_id.GetUniqueID(), transmissionBytes);
				Mem::Serialize(static_cast<uint8_t>(ComponentTransmissionType::Added), transmissionBytes);
				serializer.m_serializeFullTransmissionFunction(*currentIter, transmissionBytes);
				
				++currentIter;
			}
		}
		
		// Serialize the invalid component ID to indicate the end of the component type.
		Mem::Serialize(ComponentID::sk_invalidUniqueID, transmissionBytes);
		
		// Copy the current component data into the buffered component data.
		bufferedComponents = currentComponents;
	}

	return transmissionBytes;
}

void EntityManager::ApplyDeltaTransmission(const Collection::Vector<uint8_t>& transmissionBytes)
{
	using namespace Internal_EntityManager;

	// The transmission is a series of updates partitioned by component type.
	// Components are assumed to be sorted by ID within each type.
	const uint8_t* iter = transmissionBytes.begin();
	const uint8_t* const iterEnd = transmissionBytes.end();

	while (iter < iterEnd)
	{
		char componentTypeBuffer[64];
		if (!Mem::DeserializeString(iter, iterEnd, componentTypeBuffer))
		{
			break;
		}

		const Util::StringHash componentTypeHash = Util::CalcHash(componentTypeBuffer);
		const auto componenVectorIter = m_components.Find(componentTypeHash);
		if (componenVectorIter == m_components.end())
		{
			break;
		}
		ComponentVector& components = componenVectorIter->second;

		const auto deserializer = m_componentFactory.FindTransmissionFunctions(componentTypeHash);

		auto componentIter = components.begin();
		const auto componentsEnd = components.end();

		while (true)
		{
			const auto maybeComponentID = Mem::DeserializeUi64(iter, iterEnd);
			if ((!maybeComponentID.second) || maybeComponentID.first == ComponentID::sk_invalidUniqueID)
			{
				break;
			}
			const uint64_t componentID = maybeComponentID.first;
			
			const auto maybeTransmissionType = Mem::DeserializeUi8(iter, iterEnd);
			if ((!maybeTransmissionType.second))
			{
				break;
			}
			const ComponentTransmissionType transmissionType{ maybeTransmissionType.first };

			while (componentIter->m_id.GetUniqueID() < componentID && (componentIter + 1) < componentsEnd)
			{
				++componentIter;
			}
			if (componentIter->m_id.GetUniqueID() != componentID)
			{
				continue;
			}
			
			switch (transmissionType)
			{
			case ComponentTransmissionType::Added:
			{
				// TODO(network)
				break;
			}
			case ComponentTransmissionType::Removed:
			{
				// TODO(network)
				break;
			}
			case ComponentTransmissionType::DeltaUpdate:
			{
				deserializer.m_applyDeltaTransmissionFunction(*componentIter, iter, iterEnd);
				break;
			}
			}
		}
	}
}

void EntityManager::AddComponentToEntity(const ComponentInfo& componentInfo, Entity& entity)
{
	const Util::StringHash componentTypeHash = componentInfo.GetTypeHash();
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

	if (!m_componentFactory.TryMakeComponent(componentInfo, componentID, componentVector))
	{
		Dev::LogWarning("Failed to create component of type [%s].", componentInfo.GetTypeName());
		return;
	}

	++m_nextComponentID;
	entity.m_components.Add(componentID);
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
