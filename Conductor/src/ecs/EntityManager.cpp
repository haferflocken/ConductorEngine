#include <ecs/EntityManager.h>

#include <ecs/Component.h>
#include <ecs/ComponentReflector.h>
#include <ecs/ComponentVector.h>
#include <ecs/ECSGroupVector.h>
#include <ecs/Entity.h>
#include <ecs/SerializedEntitiesAndComponents.h>
#include <ecs/System.h>

#include <collection/ArrayView.h>
#include <mem/Serialize.h>
#include <mem/SerializeLittleEndian.h>

#include <algorithm>
#include <execution>
#include <set>

namespace ECS
{
namespace Internal_EntityManager
{
template <typename EntitiesView>
void FullySerializeEntitiesAndComponents(
	const ECS::ComponentReflector& componentReflector,
	const ECS::EntityManager& entityManager,
	const EntitiesView& entities,
	SerializedEntitiesAndComponents& serialization)
{
	// Sort the components to serialize by type and ID.
	struct ComponentIDWithPointer
	{
		ComponentID m_id;
		const Component* m_component;
	};
	Collection::VectorMap<ComponentType, Collection::Vector<ComponentIDWithPointer>> componentsToSerializeByType;
	for (const auto& entity : entities)
	{
		for (const auto& componentID : entity->GetComponentIDs())
		{
			const Component* const component = entityManager.FindComponent(componentID);
			componentsToSerializeByType[componentID.GetType()].Add({ componentID, component });
		}
	}

	for (auto& entry : componentsToSerializeByType)
	{
		Collection::Vector<ComponentIDWithPointer>& components = entry.second;
		std::sort(components.begin(), components.end(),
			[](const auto& lhs, const auto& rhs) { return lhs.m_id < rhs.m_id; });
	}

	// Serialize the component data.
	for (const auto& entry : componentsToSerializeByType)
	{
		const ComponentType componentType = entry.first;
		const auto& components = entry.second;

		const auto componentFunctions = componentReflector.FindComponentFunctions(componentType);

		auto& componentViews = serialization.m_componentViews[componentType];

		for (const auto& componentIDWithPointer : components)
		{
			const Component& component = *componentIDWithPointer.m_component;
			const uint32_t componentViewBeginIndex = serialization.m_bytes.Size();

			FullSerializedComponentHeader componentHeader;
			componentHeader.m_uniqueID = component.m_id.GetUniqueID();

			serialization.m_bytes.Resize(serialization.m_bytes.Size() + FullSerializedComponentHeader::k_unpaddedSize);
			memcpy(serialization.m_bytes.begin() + componentViewBeginIndex,
				&componentHeader,
				FullSerializedEntityHeader::k_unpaddedSize);

			componentFunctions.m_fullSerializationFunction(component, serialization.m_bytes);

			componentViews.Add({ componentViewBeginIndex, serialization.m_bytes.Size() });
		}
	}

	// Serialize the entity data.
	for (const auto& entity : entities)
	{
		const uint32_t entityViewBeginIndex = serialization.m_bytes.Size();

		FullSerializedEntityHeader entityHeader;
		entityHeader.m_entityID = entity->GetID();
		if (entity->GetParent() != nullptr)
		{
			entityHeader.m_parentEntityID = entity->GetParent()->GetID();
		}
		entityHeader.m_numComponents = entity->GetComponentIDs().Size();

		serialization.m_bytes.Resize(serialization.m_bytes.Size() + FullSerializedEntityHeader::k_unpaddedSize);
		memcpy(serialization.m_bytes.begin() + entityViewBeginIndex,
			&entityHeader,
			FullSerializedEntityHeader::k_unpaddedSize);

		for (const auto& componentID : entity->GetComponentIDs())
		{
			Mem::LittleEndian::Serialize(componentID.GetUniqueID(), serialization.m_bytes);
		}

		serialization.m_entityViews.Add({ entityViewBeginIndex, serialization.m_bytes.Size() });
	}
}
}

EntityManager::EntityManager(Asset::AssetManager& assetManager, const ComponentReflector& componentReflector)
	: m_assetManager(assetManager)
	, m_componentReflector(componentReflector)
	, m_entities(EntityIDHashFunctor(), 7)
{
}

EntityManager::~EntityManager()
{
	// Allow this EntityManager's systems to clean up using it.
	for (auto& group : m_concurrentSystemGroups)
	{
		for (auto& registeredSystem : group.m_systems)
		{
			registeredSystem.m_system->NotifyOfShutdown(*this);
		}
	}
	m_concurrentSystemGroups.Clear();
}


Entity& EntityManager::CreateEntityWithComponents(const Collection::ArrayView<const ComponentType>& componentTypes,
	const EntityID requestedID)
{
	// Determine the entity's ID.
	EntityID entityID;
	if (requestedID == EntityID())
	{
		entityID = m_nextEntityID;
		m_nextEntityID = EntityID(entityID.GetUniqueID() + 1);
	}
	else
	{
		entityID = requestedID;
		if (m_nextEntityID.GetUniqueID() <= requestedID.GetUniqueID())
		{
			m_nextEntityID = EntityID(requestedID.GetUniqueID() + 1);
		}
	}

	// Create the entity.
	Entity& entity = m_entities.Emplace(entityID, entityID);

	// Create the entity's components.
	for (const auto& componentType : componentTypes)
	{
		AddComponentToEntity(componentType, entity);
	}

	// Add the entity to the system execution groups.
	AddECSPointersToSystems(entity);

	// Return the entity after it is fully initialized.
	return entity;
}

Collection::Vector<Entity*> EntityManager::CreateEntitiesFromFullSerialization(
	const SerializedEntitiesAndComponents& serialization)
{
	// Create all serialized components.
	for (const auto& entry : serialization.m_componentViews)
	{
		ComponentVector* const componentVector = GetComponentVector(entry.first);
		if (componentVector == nullptr)
		{
			continue;
		}

		const auto componentFunctions = m_componentReflector.FindComponentFunctions(entry.first);

		for (const auto& componentView : entry.second)
		{
			const uint8_t* const viewBytes = &serialization.m_bytes[componentView.m_beginIndex];

			FullSerializedComponentHeader header;
			memcpy(&header, viewBytes, FullSerializedComponentHeader::k_unpaddedSize);

			const ComponentID componentID{ entry.first, header.m_uniqueID };

			const uint8_t* componentBegin = viewBytes + FullSerializedComponentHeader::k_unpaddedSize;
			const uint8_t* const componentEnd = viewBytes + componentView.m_endIndex;

			Component& component = componentFunctions.m_basicConstructFunction(componentID, *componentVector);
			componentFunctions.m_applyFullSerializationFunction(
				m_assetManager, component, componentBegin, componentEnd);
		}
	}

	// Create all serialized entities.
	Collection::Vector<Entity*> newEntities;
	for (const auto& entityView : serialization.m_entityViews)
	{
		const uint8_t* const viewBytes = &serialization.m_bytes[entityView.m_beginIndex];
		const size_t viewSizeInBytes = (entityView.m_endIndex - entityView.m_beginIndex);

		FullSerializedEntityHeader header;
		memcpy(&header, viewBytes, FullSerializedEntityHeader::k_unpaddedSize);

		const size_t numComponentIDBytes = header.m_numComponents * sizeof(ComponentID);
		if ((numComponentIDBytes + FullSerializedEntityHeader::k_unpaddedSize) > viewSizeInBytes)
		{
			AMP_LOG_WARNING("Serialized entity view isn't large enough for all its component IDs.");
			continue;
		}

		Collection::Vector<ComponentID> componentIDs;
		componentIDs.Resize(header.m_numComponents);

		const uint8_t* const serializedComponentIDs = viewBytes + FullSerializedEntityHeader::k_unpaddedSize;
		memcpy(&componentIDs.Front(), serializedComponentIDs, numComponentIDBytes);

		// TODO(ecs) handle entity IDs which are already in use, perhaps with a fixup map

		Entity& entity = m_entities.Emplace(header.m_entityID, header.m_entityID);
		entity.m_componentIDs = std::move(componentIDs);

		Entity* const parentEntity = FindEntity(header.m_parentEntityID);
		SetParentEntity(entity, parentEntity);

		newEntities.Add(&entity);
	}

	// Add all new entities to the system execution groups.
	AddECSPointersToSystems(newEntities.GetConstView());

	return newEntities;
}

void EntityManager::FullySerializeEntitiesAndComponents(const Collection::ArrayView<const Entity*>& entities,
	SerializedEntitiesAndComponents& serialization) const
{
	Internal_EntityManager::FullySerializeEntitiesAndComponents(m_componentReflector, *this, entities, serialization);
}

void EntityManager::FullySerializeAllEntitiesAndComponents(SerializedEntitiesAndComponents& serialization) const
{
	Internal_EntityManager::FullySerializeEntitiesAndComponents(
		m_componentReflector, *this, m_entities.GetValueView(), serialization);
}

void EntityManager::SetParentEntity(Entity& entity, Entity* parentEntity)
{
	// Detach the entity from its existing parent, if it has one.
	if (entity.m_parent != nullptr)
	{
		Entity& parent = *entity.m_parent;
		const size_t indexInParent = parent.m_children.IndexOf(&entity);
		parent.m_children.SwapWithAndRemoveLast(indexInParent);
	}

	// Attach the entity to the given parent.
	entity.m_parent = parentEntity;
	if (parentEntity != nullptr)
	{
		parentEntity->m_children.Add(&entity);
	}
}

void EntityManager::DeleteEntities(const Collection::ArrayView<const EntityID>& entitiesToDelete)
{
	// Detach the entities from their parents and delete the entities.
	// Recursively traverse their children to delete them as well.
	Collection::Vector<Entity*> allEntitiesToDelete;
	for (const auto& entityID : entitiesToDelete)
	{
		Entity* const entity = m_entities.Find(entityID);
		if (entity == nullptr)
		{
			continue;
		}

		if (entity->m_parent != nullptr)
		{
			Entity& parent = *entity->m_parent;
			const size_t indexInParent = parent.m_children.IndexOf(entity);
			parent.m_children.SwapWithAndRemoveLast(indexInParent);
		}
		allEntitiesToDelete.Add(entity);
	}

	while (!allEntitiesToDelete.IsEmpty())
	{
		Entity& entity = *allEntitiesToDelete.Back();
		allEntitiesToDelete.RemoveLast();

		// Queue the children not already in entitiesToDelete for removal.
		for (const auto& child : entity.m_children)
		{
			bool shouldQueue = true;
			for (const auto& requestedID : entitiesToDelete)
			{
				if (child->m_id == requestedID)
				{
					shouldQueue = false;
					break;
				}
			}
			if (shouldQueue)
			{
				allEntitiesToDelete.Add(child);
			}
		}

		// Delete the entity.
		RemoveECSPointersFromSystems(entity);

		for (const auto& componentID : entity.GetComponentIDs())
		{
			RemoveComponent(componentID);
		}
		m_entities.TryRemove(entity.GetID());
	}
}

Entity* EntityManager::FindEntity(const EntityID id)
{
	return m_entities.Find(id);
}

const Entity* EntityManager::FindEntity(const EntityID id) const
{
	return m_entities.Find(id);
}

Component* EntityManager::FindComponent(const ComponentID id)
{
	// Implemented using the const variant.
	return const_cast<Component*>(static_cast<const EntityManager*>(this)->FindComponent(id));
}

const Component* EntityManager::FindComponent(const ComponentID id) const
{
	const Collection::Pair<const ComponentType, ComponentVector>* const componentsEntry =
		m_components.Find(id.GetType());
	if (componentsEntry == m_components.end())
	{
		return nullptr;
	}
	const ComponentVector& components = componentsEntry->second;
	return components.Find(id);
}

ComponentVector* EntityManager::GetComponentVector(const ComponentType componentType)
{
	// Components with size 0 are tag components and are never instantiated.
	const Unit::ByteCount64 componentSize = m_componentReflector.GetSizeOfComponentInBytes(componentType);
	if (componentSize.GetN() == 0)
	{
		return nullptr;
	}

	ComponentVector& componentVector = m_components[componentType];
	if (componentVector.GetComponentType() == ComponentType())
	{
		// This is a type that has not yet been encountered and therefore must be initialized.
		const Unit::ByteCount64 componentAlignment = m_componentReflector.GetAlignOfComponentInBytes(componentType);

		componentVector = ComponentVector(m_componentReflector, componentType, componentSize, componentAlignment);
	}
	AMP_FATAL_ASSERT(componentVector.GetComponentType() == componentType,
		"Mismatch between component vector type and the key it is stored at.");

	return &componentVector;
}

void EntityManager::AddComponentToEntity(const ComponentType componentType, Entity& entity)
{
	const ComponentID componentID{ componentType, m_nextComponentID };

	ComponentVector* const componentVector = GetComponentVector(componentType);
	if (componentVector != nullptr)
	{
		if (!m_componentReflector.TryBasicConstructComponent(componentID, *componentVector))
		{
			AMP_LOG_WARNING("Failed to create component of type [%s].", Util::ReverseHash(componentType.GetTypeHash()));
			return;
		}
	}

	++m_nextComponentID;
	entity.m_componentIDs.Add(componentID);
}

void EntityManager::AddComponentToEntity(
	const ComponentType componentType, const uint8_t*& bytes, const uint8_t* bytesEnd, Entity& entity)
{
	const ComponentID componentID{ componentType, m_nextComponentID };

	ComponentVector* const componentVector = GetComponentVector(componentType);
	if (componentVector != nullptr)
	{
		if (!m_componentReflector.TryMakeComponent(m_assetManager, bytes, bytesEnd, componentID, *componentVector))
		{
			AMP_LOG_WARNING("Failed to create component of type [%s].", Util::ReverseHash(componentType.GetTypeHash()));
			return;
		}
	}

	++m_nextComponentID;
	entity.m_componentIDs.Add(componentID);
}

void EntityManager::RemoveComponent(const ComponentID id)
{
	// Delete the component.
	Collection::Pair<const ComponentType, ComponentVector>* const componentsEntry =
		m_components.Find(id.GetType());
	ComponentVector& components = componentsEntry->second;
	components.Remove(id);
}

EntityManager::RegisteredSystem::RegisteredSystem()
	: m_system()
	, m_updateFunction(nullptr)
	, m_notifyEntityAddedFunction(nullptr)
	, m_notifyEntityRemovedFunction(nullptr)
	, m_ecsGroups()
	, m_deferredFunctions()
{}

EntityManager::RegisteredSystem::RegisteredSystem(
	Mem::UniquePtr<System>&& system,
	SystemUpdateFn updateFunction,
	NotifyOfEntityFn notifyEntityAddedFunction,
	NotifyOfEntityFn notifyEntityRemovedFunction)
	: m_system(std::move(system))
	, m_updateFunction(updateFunction)
	, m_notifyEntityAddedFunction(notifyEntityAddedFunction)
	, m_notifyEntityRemovedFunction(notifyEntityRemovedFunction)
	, m_ecsGroups(m_system->GetImmutableTypes().Size() + m_system->GetMutableTypes().Size())
{}

namespace Internal_EntityManager
{
bool TryGatherPointers(EntityManager& entityManager, const Collection::Vector<ECS::ComponentType>& componentTypes,
	Entity& entity, Collection::Vector<void*>& pointers)
{
	bool foundAll = true;
	for (const auto& ecsType : componentTypes)
	{
		if (ecsType == Entity::k_type)
		{
			pointers.Add(&entity);
			continue;
		}

		const ComponentID id = entity.FindComponentID(ecsType);
		if (id == ComponentID())
		{
			foundAll = false;
			break;
		}
		pointers.Add(entityManager.FindComponent(id));
	}
	return foundAll;
};
}

void EntityManager::AddECSPointersToSystems(Entity& entityToAdd)
{
	Entity* const entityPtr = &entityToAdd;
	AddECSPointersToSystems(Collection::ArrayView<Entity* const>{ &entityPtr, 1 });
}

void EntityManager::AddECSPointersToSystems(Collection::ArrayView<Entity* const> entitiesToAdd)
{
	using namespace Internal_EntityManager;

	for (auto& executionGroup : m_concurrentSystemGroups)
	{
		// Gather the entity pointers and component pointers each member of the execution group needs.
		for (auto& registeredSystem : executionGroup.m_systems)
		{
			const Collection::Vector<ECS::ComponentType>& immutableTypes =
				registeredSystem.m_system->GetImmutableTypes();
			const Collection::Vector<ECS::ComponentType>& mutableTypes =
				registeredSystem.m_system->GetMutableTypes();

			for (auto& entity : entitiesToAdd)
			{
				Collection::Vector<void*> pointers;
				if (!TryGatherPointers(*this, immutableTypes, *entity, pointers))
				{
					continue;
				}
				if (!TryGatherPointers(*this, mutableTypes, *entity, pointers))
				{
					continue;
				}

				registeredSystem.m_ecsGroups.Add(pointers);
				registeredSystem.m_notifyEntityAddedFunction(registeredSystem, entity->GetID(), pointers);
			}

			// Sort the group vector in order to make the memory accesses as fast as possible.
			registeredSystem.m_ecsGroups.Sort();
		}
	}
}

void EntityManager::RemoveECSPointersFromSystems(Entity& entity)
{
	using namespace Internal_EntityManager;

	for (auto& executionGroup : m_concurrentSystemGroups)
	{
		for (auto& registeredSystem : executionGroup.m_systems)
		{
			const Collection::Vector<ECS::ComponentType>& immutableTypes =
				registeredSystem.m_system->GetImmutableTypes();
			const Collection::Vector<ECS::ComponentType>& mutableTypes =
				registeredSystem.m_system->GetMutableTypes();

			Collection::Vector<void*> pointers;
			if (!TryGatherPointers(*this, immutableTypes, entity, pointers))
			{
				continue;
			}
			if (!TryGatherPointers(*this, mutableTypes, entity, pointers))
			{
				continue;
			}

			registeredSystem.m_ecsGroups.Remove(pointers);
			registeredSystem.m_notifyEntityRemovedFunction(registeredSystem, entity.GetID(), pointers);
		}
	}
}

void EntityManager::Update(const Unit::Time::Millisecond delta)
{
	// Update the concurrent system groups.
	for (auto& concurrentGroup : m_concurrentSystemGroups)
	{
		// The systems in each group can update in parallel.
		// If the group has only one system, run it directly on this thread.
		if (concurrentGroup.m_systems.Size() == 1)
		{
			concurrentGroup.m_systems.Front().m_updateFunction(concurrentGroup.m_systems.Front(), delta);
		}
		else
		{
			std::for_each(std::execution::par, concurrentGroup.m_systems.begin(), concurrentGroup.m_systems.end(),
				[&](RegisteredSystem& registeredSystem)
				{
					registeredSystem.m_updateFunction(registeredSystem, delta);
				});
		}

		// Resolve deferred functions single-threaded.
		for (auto& registeredSystem : concurrentGroup.m_systems)
		{
			for (auto& deferredFunction : registeredSystem.m_deferredFunctions)
			{
				deferredFunction(*this);
			}
			registeredSystem.m_deferredFunctions.Clear();
		}
	}
}
}
