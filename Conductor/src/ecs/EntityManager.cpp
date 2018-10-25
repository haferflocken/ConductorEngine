#include <ecs/EntityManager.h>

#include <ecs/Component.h>
#include <ecs/ComponentReflector.h>
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
enum class TransmissionSectionType : uint8_t
{
	ComponentsAdded = 0,
	ComponentsRemoved,
	ComponentsDeltaUpdate,
	EntitiesAdded,
	EntitiesRemoved,
	EntitiesChanged,
};
}

EntityManager::EntityManager(Asset::AssetManager& assetManager, const ComponentReflector& componentReflector,
	bool transmitsState)
	: m_assetManager(assetManager)
	, m_componentReflector(componentReflector)
	, m_entities(EntityIDHashFunctor(), 7)
{
	if (transmitsState)
	{
		m_transmissionBuffers = Mem::MakeUnique<TransmissionBuffers>();
	}
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

Entity& EntityManager::CreateEntity(const EntityInfo& entityInfo)
{
	// Create the entity.
	const EntityID entityID = m_nextEntityID;
	
	Entity& entity = m_entities.Emplace(entityID, entityID, entityInfo.m_nameHash);

	// Create the entity's components using our component reflector.
	for (const auto& componentInfo : entityInfo.m_componentInfos)
	{
		AddComponentToEntity(*componentInfo, entity);
	}
	
	// Update the next unique ID.
	m_nextEntityID = EntityID(entityID.GetUniqueID() + 1);

	// Add the entity to the system execution groups.
	AddECSPointersToSystems(Collection::ArrayView<Entity>(&entity, 1));

	// If we can transmit state, track the newly added entity.
	if (m_transmissionBuffers != nullptr)
	{
		m_transmissionBuffers->m_entitiesAddedSinceLastTransmission.Add(entityID);
	}

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

	// Remove the entity from all ECS groups.
	RemoveECSPointersFromSystems(entity);

	// Remove any components the entity no longer needs.
	for (size_t i = 0; i < entity.m_components.Size();)
	{
		const ComponentID& componentID = entity.m_components[i];

		const auto* const matchingComponentInfo = entityInfo.m_componentInfos.Find(
			[&](const Mem::UniquePtr<ComponentInfo>& componentInfo)
		{
			return componentInfo->GetTypeHash() == componentID.GetType().GetTypeHash();
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
		const ComponentID componentID = entity.FindComponentID(ComponentType(componentInfo->GetTypeHash()));
		if (componentID != ComponentID())
		{
			continue;
		}
		AddComponentToEntity(*componentInfo, entity);
	}

	// Add the entity to the ECS group vectors.
	AddECSPointersToSystems(Collection::ArrayView<Entity>(&entity, 1));

	// If we can transmit state, track the changed entity.
	if (m_transmissionBuffers != nullptr)
	{
		m_transmissionBuffers->m_entitiesChangedSinceLastTransmission.Add(entity.GetID());
	}
}

void EntityManager::DeleteEntities(const Collection::ArrayView<const EntityID>& entitiesToDelete)
{
	// If we can transmit state, track the entities that are removed.
	if (m_transmissionBuffers != nullptr)
	{
		for (const auto& entityID : entitiesToDelete)
		{
			m_transmissionBuffers->m_entitiesRemovedSinceLastTransmission.Add(entityID);
		}
	}

	// Delete the entities.
	for (const auto& entityID : entitiesToDelete)
	{
		Entity* const entity = m_entities.Find(entityID);
		if (entity == nullptr)
		{
			continue;
		}

		RemoveECSPointersFromSystems(*entity);

		for (const auto& componentID : entity->GetComponentIDs())
		{
			RemoveComponent(componentID);
		}
		m_entities.TryRemove(entityID);
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

Collection::Vector<uint8_t> EntityManager::SerializeDeltaTransmission()
{
	using namespace Internal_EntityManager;
	AMP_FATAL_ASSERT(m_transmissionBuffers != nullptr,
		"Cannot serialize an EntityManager that was not flagged at creation to support transmission.");

	Collection::Vector<uint8_t> transmissionBytes;

	// Serialize components that were removed.
	if (!m_transmissionBuffers->m_componentsRemovedSinceLastTransmission.IsEmpty())
	{
		// First, sort them by type and within each type by ID.
		std::sort(m_transmissionBuffers->m_componentsAddedSinceLastTransmission.begin(),
			m_transmissionBuffers->m_componentsAddedSinceLastTransmission.end());

		// Serialize the IDs within each type.
		auto iter = m_transmissionBuffers->m_componentsRemovedSinceLastTransmission.begin();
		const auto iterEnd = m_transmissionBuffers->m_componentsRemovedSinceLastTransmission.end();
		
		while (iter < iterEnd)
		{
			const ComponentType componentType = iter->GetType();

			Mem::Serialize(static_cast<uint8_t>(TransmissionSectionType::ComponentsRemoved), transmissionBytes);
			Mem::Serialize(Util::ReverseHash(componentType.GetTypeHash()), transmissionBytes);

			while (iter < iterEnd && iter->GetType() == componentType)
			{
				Mem::Serialize(iter->GetUniqueID(), transmissionBytes);
				++iter;
			}

			// Serialize the invalid component ID to indicate the end of the component type.
			Mem::Serialize(ComponentID::sk_invalidUniqueID, transmissionBytes);
		}
	}

	// Serialize the component delta state changes.
	for (auto& entry : m_transmissionBuffers->m_bufferedComponents)
	{
		// Serialize a marker and the component type string to start the delta update for this component type.
		Mem::Serialize(static_cast<uint8_t>(TransmissionSectionType::ComponentsDeltaUpdate), transmissionBytes);
		Mem::Serialize(Util::ReverseHash(entry.first.GetTypeHash()), transmissionBytes);

		const auto& serializer = *m_componentReflector.FindTransmissionFunctions(entry.first);

		ComponentVector& bufferedComponents = entry.second;
		const ComponentVector& currentComponents = m_components.Find(entry.first)->second;

		auto bufferedIter = bufferedComponents.begin();
		auto currentIter = currentComponents.begin();
		const auto bufferedEnd = bufferedComponents.end();
		const auto currentEnd = currentComponents.end();

		for (const auto& rawComponent : currentComponents)
		{
			const Component& component = reinterpret_cast<const Component&>(rawComponent);
			const Component* const bufferedComponent = bufferedComponents.Find(component.m_id);
			if (bufferedComponent == nullptr)
			{
				continue;
			}
			Mem::Serialize(bufferedComponent->m_id.GetUniqueID(), transmissionBytes);
			serializer.m_serializeDeltaTransmissionFunction(*bufferedComponent, component, transmissionBytes);
		}
		
		// Serialize the invalid component ID to indicate the end of the component type.
		Mem::Serialize(ComponentID::sk_invalidUniqueID, transmissionBytes);
		
		// Copy the current component data into the buffered component data.
		bufferedComponents.Copy(currentComponents);
	}

	// Serialize the components that were added.
	if (!m_transmissionBuffers->m_componentsAddedSinceLastTransmission.IsEmpty())
	{
		// First, sort them by type and within each type by ID.
		std::sort(m_transmissionBuffers->m_componentsAddedSinceLastTransmission.begin(),
			m_transmissionBuffers->m_componentsAddedSinceLastTransmission.end());

		// Serialize the components within each type.
		auto iter = m_transmissionBuffers->m_componentsAddedSinceLastTransmission.begin();
		const auto iterEnd = m_transmissionBuffers->m_componentsAddedSinceLastTransmission.end();

		while (iter < iterEnd)
		{
			const ComponentType componentType = iter->GetType();
			const auto* const serializer = m_componentReflector.FindTransmissionFunctions(componentType);

			Mem::Serialize(static_cast<uint8_t>(TransmissionSectionType::ComponentsAdded), transmissionBytes);
			Mem::Serialize(Util::ReverseHash(componentType.GetTypeHash()), transmissionBytes);

			while (iter < iterEnd && iter->GetType() == componentType)
			{
				// TODO(network) this could be optimized by only getting the component vector once per type
				// and scanning the sorted components in the vector.
				// The component may have been added and removed since the last transmission;
				// in this case it does not need to be serialized.
				const Component* const component = FindComponent(*iter);
				if (component != nullptr)
				{
					Mem::Serialize(iter->GetUniqueID(), transmissionBytes);
					if (serializer != nullptr)
					{
						serializer->m_serializeFullTransmissionFunction(*component, transmissionBytes);
					}
				}
				++iter;
			}

			// Serialize the invalid component ID to indicate the end of the component type.
			Mem::Serialize(ComponentID::sk_invalidUniqueID, transmissionBytes);
		}
	}

	// Serialize entities that were removed.
	Mem::Serialize(static_cast<uint8_t>(TransmissionSectionType::EntitiesRemoved), transmissionBytes);
	for (const auto& entityID : m_transmissionBuffers->m_entitiesRemovedSinceLastTransmission)
	{
		Mem::Serialize(entityID.GetUniqueID(), transmissionBytes);
	}
	Mem::Serialize(EntityID::sk_invalidValue, transmissionBytes);

	// Serialize entities that changed or were added.
	const auto SerializeEntitySection = [this](const TransmissionSectionType sectionType,
		const Collection::Vector<EntityID>& entityIDs, Collection::Vector<uint8_t>& transmissionBytes)
	{
		Mem::Serialize(static_cast<uint8_t>(sectionType), transmissionBytes);
		for (const auto& entityID : entityIDs)
		{
			const Entity* const entity = FindEntity(entityID);
			if (entity == nullptr)
			{
				// An entity can be null if it was added and removed since the last transmission.
				continue;
			}

			Mem::Serialize(entityID.GetUniqueID(), transmissionBytes);
			Mem::Serialize(Util::ReverseHash(entity->GetInfoNameHash()), transmissionBytes);
			Mem::Serialize(entity->GetComponentIDs().Size(), transmissionBytes);
			for (const auto& componentID : entity->GetComponentIDs())
			{
				Mem::Serialize(componentID.GetUniqueID(), transmissionBytes);
			}
		}
		Mem::Serialize(EntityID::sk_invalidValue, transmissionBytes);
	};

	SerializeEntitySection(TransmissionSectionType::EntitiesChanged,
		m_transmissionBuffers->m_entitiesChangedSinceLastTransmission, transmissionBytes);

	SerializeEntitySection(TransmissionSectionType::EntitiesAdded,
		m_transmissionBuffers->m_entitiesAddedSinceLastTransmission, transmissionBytes);

	// Clear the transmission buffers after the transmission is serialized.
	m_transmissionBuffers->m_entitiesAddedSinceLastTransmission.Clear();
	m_transmissionBuffers->m_entitiesChangedSinceLastTransmission.Clear();
	m_transmissionBuffers->m_entitiesRemovedSinceLastTransmission.Clear();
	m_transmissionBuffers->m_componentsAddedSinceLastTransmission.Clear();
	m_transmissionBuffers->m_componentsRemovedSinceLastTransmission.Clear();

	return transmissionBytes;
}

ECS::ApplyDeltaTransmissionResult EntityManager::ApplyDeltaTransmission(
	const Collection::Vector<uint8_t>& transmissionBytes)
{
	using namespace Internal_EntityManager;

	// The transmission consists of these sections:
	// - Components removed (one section per required component type)
	// - Components changed (one section per required component type)
	// - Components added (one section per required component type)
	// - Entities removed
	// - Entities changed
	// - Entities added

	// The transmission is a series of updates partitioned by component type.
	// Components are assumed to be sorted by ID within each type.
	const uint8_t* iter = transmissionBytes.begin();
	const uint8_t* const iterEnd = transmissionBytes.end();

	auto maybeSectionMarker = Mem::DeserializeUi8(iter, iterEnd);
	if (!maybeSectionMarker.second)
	{
		return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_DataTooShort>();
	}
	TransmissionSectionType sectionMarker{ maybeSectionMarker.first };

	const auto ComponentSectionHelper = [this, &iter, &iterEnd, &sectionMarker](
		const TransmissionSectionType sectionType, auto&& work)
	{
		while (sectionMarker == sectionType)
		{
			char componentTypeBuffer[64];
			if (!Mem::DeserializeString(iter, iterEnd, componentTypeBuffer))
			{
				return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_DataTooShort>();
			}

			const Util::StringHash componentTypeHash = Util::CalcHash(componentTypeBuffer);
			const auto componenVectorIter = m_components.Find(ComponentType(componentTypeHash));
			if (componenVectorIter == m_components.end())
			{
				return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_UnrecognizedComponentType>(
					componentTypeBuffer);
			}
			ComponentVector& components = componenVectorIter->second;

			const ApplyDeltaTransmissionResult result = work(
				m_componentReflector, ComponentType(componentTypeHash), components, iter, iterEnd);
			if (result.IsAny())
			{
				return result;
			}

			const auto maybeSectionMarker = Mem::DeserializeUi8(iter, iterEnd);
			if (!maybeSectionMarker.second)
			{
				return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_DataTooShort>();
			}
			sectionMarker = TransmissionSectionType{ maybeSectionMarker.first };
		}

		return ApplyDeltaTransmissionResult();
	};

	auto result = ComponentSectionHelper(TransmissionSectionType::ComponentsRemoved,
		[](const ComponentReflector& componentReflector, const ComponentType componentType,
			ComponentVector& components, const uint8_t*& iter, const uint8_t* iterEnd)
		{
			Collection::Vector<uint64_t> componentIDsToRemove;
			auto maybeComponentID = Mem::DeserializeUi64(iter, iterEnd);
			while (maybeComponentID.second && maybeComponentID.first != ComponentID::sk_invalidUniqueID)
			{
				componentIDsToRemove.Add(maybeComponentID.first);
				maybeComponentID = Mem::DeserializeUi64(iter, iterEnd);
			}

			std::sort(componentIDsToRemove.begin(), componentIDsToRemove.end());
			components.RemoveSorted(componentIDsToRemove.GetConstView());

			return ApplyDeltaTransmissionResult();
		});
	if (result.IsAny())
	{
		return result;
	}

	result = ComponentSectionHelper(TransmissionSectionType::ComponentsDeltaUpdate,
		[](const ComponentReflector& componentReflector, const ComponentType componentType,
			ComponentVector& components, const uint8_t*& iter, const uint8_t* iterEnd)
		{
			const auto* const deserializer = componentReflector.FindTransmissionFunctions(componentType);
			if (deserializer == nullptr)
			{
				return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_UnrecognizedComponentType>(
					Util::ReverseHash(componentType.GetTypeHash()));
			}

			auto maybeComponentID = Mem::DeserializeUi64(iter, iterEnd);
			while (maybeComponentID.second && maybeComponentID.first != ComponentID::sk_invalidUniqueID)
			{
				const uint64_t componentID = maybeComponentID.first;
				Component* const component = components.Find(ComponentID(componentType, componentID));
				if (component != nullptr)
				{
					deserializer->m_applyDeltaTransmissionFunction(*component, iter, iterEnd);
				}
				maybeComponentID = Mem::DeserializeUi64(iter, iterEnd);
			}

			return ApplyDeltaTransmissionResult();
		});
	if (result.IsAny())
	{
		return result;
	}

	result = ComponentSectionHelper(TransmissionSectionType::ComponentsAdded, 
		[](const ComponentReflector& componentReflector, const ComponentType componentType,
			ComponentVector& components, const uint8_t*& iter, const uint8_t* iterEnd)
		{
			const auto* const deserializer = componentReflector.FindTransmissionFunctions(componentType);

			auto maybeComponentID = Mem::DeserializeUi64(iter, iterEnd);
			while (maybeComponentID.second && maybeComponentID.first != ComponentID::sk_invalidUniqueID)
			{
				const uint64_t componentID = maybeComponentID.first;
				//if ((!components.IsEmpty()) && components[components.Size() - 1].m_id.GetUniqueID() >= componentID)
				//{
				//	return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_ComponentAddedOutOfOrder>();
				//}

				// TODO(network) what should be done if a component fails to be created?
				if (deserializer != nullptr)
				{
					deserializer->m_tryCreateFromTransmissionFunction(iter, iterEnd,
						ComponentID(componentType, componentID), components);
				}
				else
				{
					// TODO(network) make the component even though it wasn't serialized
				}

				maybeComponentID = Mem::DeserializeUi64(iter, iterEnd);
			}

			return ApplyDeltaTransmissionResult();
		});
	if (result.IsAny())
	{
		return result;
	}

	// Deserialize entity removals.
	if (sectionMarker == TransmissionSectionType::EntitiesRemoved)
	{
		Collection::Vector<EntityID> entitiesToRemove;

		auto maybeEntityID = Mem::DeserializeUi32(iter, iterEnd);
		while (maybeEntityID.second && maybeEntityID.first != EntityID::sk_invalidValue)
		{
			const EntityID entityID{ maybeEntityID.first };
			entitiesToRemove.Add(entityID);

			maybeEntityID = Mem::DeserializeUi32(iter, iterEnd);
		}

		DeleteEntities(entitiesToRemove.GetConstView());
		
		// Read the next section marker.
		maybeSectionMarker = Mem::DeserializeUi8(iter, iterEnd);
		if (!maybeSectionMarker.second)
		{
			return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_DataTooShort>();
		}
		sectionMarker = TransmissionSectionType{ maybeSectionMarker.first };
	}

	// Deserialize entities that changed.
	if (sectionMarker == TransmissionSectionType::EntitiesChanged)
	{
		auto maybeEntityID = Mem::DeserializeUi32(iter, iterEnd);
		while (maybeEntityID.second && maybeEntityID.first != EntityID::sk_invalidValue)
		{
			const EntityID entityID{ maybeEntityID.first };

			char infoNameBuffer[64];
			if (!Mem::DeserializeString(iter, iterEnd, infoNameBuffer))
			{
				return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_DataTooShort>();
			}
			const Util::StringHash infoNameHash = Util::CalcHash(infoNameBuffer);

			const auto maybeNumComponents = Mem::DeserializeUi32(iter, iterEnd);
			if (!maybeNumComponents.second)
			{
				return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_DataTooShort>();
			}
			const uint32_t numComponents = maybeNumComponents.first;

			Entity* const entity = FindEntity(entityID);
			if (entity != nullptr)
			{
				entity->m_components.Clear();
			}

			for (size_t i = 0; i < numComponents; ++i)
			{
				const auto maybeComponentID = Mem::DeserializeUi64(iter, iterEnd);
				if (!maybeComponentID.second)
				{
					return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_DataTooShort>();
				}

				if (entity == nullptr)
				{
					continue;
				}

				// Determine the type of the component by searching for it.
				for (const auto& entry : m_components)
				{
					bool found = false;
					for (const auto& rawComponent : entry.second)
					{
						const Component& component = reinterpret_cast<const Component&>(rawComponent);
						if (component.m_id.GetUniqueID() == maybeComponentID.first)
						{
							found = true;
							break;
						}
					}
					if (found)
					{
						entity->m_components.Add(ComponentID(entry.first, maybeComponentID.first));
						break;
					}
				}
			}

			maybeEntityID = Mem::DeserializeUi32(iter, iterEnd);
		}

		// Read the next section marker.
		maybeSectionMarker = Mem::DeserializeUi8(iter, iterEnd);
		if (!maybeSectionMarker.second)
		{
			return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_DataTooShort>();
		}
		sectionMarker = TransmissionSectionType{ maybeSectionMarker.first };
	}

	// Deserialize new entities.
	if (sectionMarker == TransmissionSectionType::EntitiesAdded)
	{
		auto maybeEntityID = Mem::DeserializeUi32(iter, iterEnd);
		while (maybeEntityID.second && maybeEntityID.first != EntityID::sk_invalidValue)
		{
			const EntityID entityID{ maybeEntityID.first };

			//if ((!m_entities.IsEmpty()) && m_entities.Back().GetID() >= entityID)
			//{
			//	return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_EntityAddedOutOfOrder>();
			//}

			char infoNameBuffer[64];
			if (!Mem::DeserializeString(iter, iterEnd, infoNameBuffer))
			{
				return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_DataTooShort>();
			}
			const Util::StringHash infoNameHash = Util::CalcHash(infoNameBuffer);

			const auto maybeNumComponents = Mem::DeserializeUi32(iter, iterEnd);
			if (!maybeNumComponents.second)
			{
				return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_DataTooShort>();
			}
			const uint32_t numComponents = maybeNumComponents.first;

			Entity& entity = m_entities.Emplace(entityID, entityID, infoNameHash);

			for (size_t i = 0; i < numComponents; ++i)
			{
				const auto maybeComponentID = Mem::DeserializeUi64(iter, iterEnd);
				if (!maybeComponentID.second)
				{
					return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_DataTooShort>();
				}

				// Determine the type of the component by searching for it.
				for (const auto& entry : m_components)
				{
					bool found = false;
					for (const auto& rawComponent : entry.second)
					{
						const Component& component = reinterpret_cast<const Component&>(rawComponent);
						if (component.m_id.GetUniqueID() == maybeComponentID.first)
						{
							found = true;
							break;
						}
					}
					if (found)
					{
						entity.m_components.Add(ComponentID(entry.first, maybeComponentID.first));
						break;
					}
				}
			}

			maybeEntityID = Mem::DeserializeUi32(iter, iterEnd);
		}

		// No need to deserialize the next section marker because there is no next section.
	}

	return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_Success>();
}

void EntityManager::AddComponentToEntity(const ComponentInfo& componentInfo, Entity& entity)
{
	const ComponentType componentType{ componentInfo.GetTypeHash() };
	const ComponentID componentID{ componentType, m_nextComponentID };

	ComponentVector& componentVector = m_components[componentType];
	if (componentVector.GetComponentType() == ComponentType())
	{
		// This is a type that has not yet been encountered and therefore must be initialized.
		const Unit::ByteCount64 componentSize = m_componentReflector.GetSizeOfComponentInBytes(componentType);
		const Unit::ByteCount64 componentAlignment = m_componentReflector.GetAlignOfComponentInBytes(componentType);

		componentVector = ComponentVector(m_componentReflector, componentType, componentSize, componentAlignment);

		if (m_transmissionBuffers != nullptr && m_componentReflector.IsNetworkedComponent(componentType))
		{
			ComponentVector& bufferedComponentVector = m_transmissionBuffers->m_bufferedComponents[componentType];
			AMP_FATAL_ASSERT(bufferedComponentVector.GetComponentType() == ComponentType(),
				"A buffered component vector was created too early.");
			bufferedComponentVector =
				ComponentVector(m_componentReflector, componentType, componentSize, componentAlignment);
		}
	}
	AMP_FATAL_ASSERT(componentVector.GetComponentType() == componentType,
		"Mismatch between component vector type and the key it is stored at.");

	if (!m_componentReflector.TryMakeComponent(m_assetManager, componentInfo, componentID, componentVector))
	{
		AMP_LOG_WARNING("Failed to create component of type [%s].", componentInfo.GetTypeName());
		return;
	}

	++m_nextComponentID;
	entity.m_components.Add(componentID);

	// If we can transmit state, track the newly added component.
	if (m_transmissionBuffers != nullptr)
	{
		m_transmissionBuffers->m_componentsAddedSinceLastTransmission.Add(componentID);
	}
}

void EntityManager::RemoveComponent(const ComponentID id)
{
	// Delete the component.
	Collection::Pair<const ComponentType, ComponentVector>* const componentsEntry =
		m_components.Find(id.GetType());
	ComponentVector& components = componentsEntry->second;
	components.Remove(id);

	// If we can transmit state, track the removed component.
	if (m_transmissionBuffers != nullptr)
	{
		m_transmissionBuffers->m_componentsRemovedSinceLastTransmission.Add(id);
	}
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
bool TryGatherPointers(EntityManager& entityManager, const Collection::Vector<Util::StringHash>& componentTypes,
	Entity& entity, Collection::Vector<void*>& pointers)
{
	bool foundAll = true;
	for (const auto& typeHash : componentTypes)
	{
		if (typeHash == EntityInfo::sk_typeHash)
		{
			pointers.Add(&entity);
			continue;
		}

		const ComponentID id = entity.FindComponentID(ComponentType(typeHash));
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

void EntityManager::AddECSPointersToSystems(Collection::ArrayView<Entity>& entitiesToAdd)
{
	using namespace Internal_EntityManager;

	for (auto& executionGroup : m_concurrentSystemGroups)
	{
		// Gather the entity pointers and component pointers each member of the execution group needs.
		for (auto& registeredSystem : executionGroup.m_systems)
		{
			const Collection::Vector<Util::StringHash>& immutableTypes =
				registeredSystem.m_system->GetImmutableTypes();
			const Collection::Vector<Util::StringHash>& mutableTypes =
				registeredSystem.m_system->GetMutableTypes();

			for (auto& entity : entitiesToAdd)
			{
				Collection::Vector<void*> pointers;
				if (!TryGatherPointers(*this, immutableTypes, entity, pointers))
				{
					continue;
				}
				if (!TryGatherPointers(*this, mutableTypes, entity, pointers))
				{
					continue;
				}

				registeredSystem.m_ecsGroups.Add(pointers);
				registeredSystem.m_notifyEntityAddedFunction(registeredSystem, entity.GetID(), pointers);
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
			const Collection::Vector<Util::StringHash>& immutableTypes =
				registeredSystem.m_system->GetImmutableTypes();
			const Collection::Vector<Util::StringHash>& mutableTypes =
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
