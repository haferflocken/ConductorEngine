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

EntityManager::EntityManager(const ComponentReflector& componentReflector, bool transmitsState)
	: m_componentReflector(componentReflector)
{
	if (transmitsState)
	{
		m_transmissionBuffers = Mem::MakeUnique<TransmissionBuffers>();
	}
}

EntityManager::~EntityManager()
{
}

Entity& EntityManager::CreateEntity(const EntityInfo& entityInfo)
{
	// Create the entity.
	const EntityID entityID = m_nextEntityID;
	Entity& entity = m_entities.Emplace(entityID, entityInfo.m_nameHash);

	// Create the entity's components using our component reflector.
	for (const auto& componentInfo : entityInfo.m_componentInfos)
	{
		AddComponentToEntity(*componentInfo, entity);
	}
	
	// Update the next unique ID.
	m_nextEntityID = EntityID(entityID.GetUniqueID() + 1);

	// Add the entity's components to the system execution groups.
	AddECSIndicesToSystems(Collection::ArrayView<Entity>(&entity, 1));

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
	const size_t removeIndex = m_entities.Partition([&](const Entity& entity)
	{
		for (const auto& entityID : entitiesToDelete)
		{
			if (entityID == entity.GetID())
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
	const Collection::Pair<const ComponentType, ComponentVector>* const componentsEntry =
		m_components.Find(id.GetType());
	if (componentsEntry == m_components.end())
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
	const Collection::Pair<const ComponentType, ComponentVector>* const componentsEntry =
		m_components.Find(id.GetType());
	if (componentsEntry == m_components.end())
	{
		return SIZE_MAX;
	}
	const ComponentVector& components = componentsEntry->second;

	const auto itr = std::lower_bound(components.begin(), components.end(), id,
		[](const Component& component, const ComponentID& id)
	{
		return component.m_id < id;
	});
	if (itr == components.end() || itr->m_id != id)
	{
		return SIZE_MAX;
	}
	return static_cast<size_t>(itr.GetIndex());
}

Entity& EntityManager::GetEntityByIndex(const size_t index)
{
	return m_entities[index];
}

Component& EntityManager::GetComponentByIndex(const ComponentType componentType, const size_t index)
{
	return m_components.Find(componentType)->second[index];
}

Collection::Vector<uint8_t> EntityManager::SerializeDeltaTransmission()
{
	using namespace Internal_EntityManager;
	Dev::FatalAssert(m_transmissionBuffers != nullptr,
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

		// Each vector of components is sorted by ID. Because of this, they can be iterated over at the same time.
		// Components are each serialized first with their ID and then with a ComponentTransmissionType.
		while (bufferedIter < bufferedEnd && currentIter < currentEnd)
		{
			while (bufferedIter->m_id < currentIter->m_id && (bufferedIter + 1) < bufferedEnd)
			{
				// If the buffered component ID is less than the current component ID,
				// the buffered component was deleted.
				++bufferedIter;
			}

			if (bufferedIter->m_id == currentIter->m_id)
			{
				// If the IDs are equal, a delta update can be made.
				Mem::Serialize(bufferedIter->m_id.GetUniqueID(), transmissionBytes);
				serializer.m_serializeDeltaTransmissionFunction(*bufferedIter, *currentIter, transmissionBytes);

				++bufferedIter;
				++currentIter;
			}
			else
			{
				// If the buffered component ID is greater than the current component ID,
				// the current component was created.
				++currentIter;
			}
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

			auto componentIter = components.begin();
			const auto componentsEnd = components.end();

			auto maybeComponentID = Mem::DeserializeUi64(iter, iterEnd);
			while (maybeComponentID.second && maybeComponentID.first != ComponentID::sk_invalidUniqueID)
			{
				const uint64_t componentID = maybeComponentID.first;

				while (componentIter->m_id.GetUniqueID() < componentID && (componentIter + 1) < componentsEnd)
				{
					++componentIter;
				}
				if (componentIter->m_id.GetUniqueID() != componentID)
				{
					continue;
				}

				deserializer->m_applyDeltaTransmissionFunction(*componentIter, iter, iterEnd);

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
				
				if ((!components.IsEmpty()) && components[components.Size() - 1].m_id.GetUniqueID() >= componentID)
				{
					return ApplyDeltaTransmissionResult::Make<ApplyDeltaTransmission_ComponentAddedOutOfOrder>();
				}

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
					for (const auto& component : entry.second)
					{
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

			if ((!m_entities.IsEmpty()) && m_entities.Back().GetID() >= entityID)
			{
				return ApplyDeltaTransmissionResult::Make< ApplyDeltaTransmission_EntityAddedOutOfOrder>();
			}

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

			Entity& entity = m_entities.Emplace(entityID, infoNameHash);

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
					for (const auto& component : entry.second)
					{
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

		componentVector = ComponentVector(m_componentReflector, componentType, componentSize);

		if (m_transmissionBuffers != nullptr && m_componentReflector.IsNetworkedComponent(componentType))
		{
			ComponentVector& bufferedComponentVector = m_transmissionBuffers->m_bufferedComponents[componentType];
			Dev::FatalAssert(bufferedComponentVector.GetComponentType() == ComponentType(),
				"A buffered component vector was created too early.");
			bufferedComponentVector = ComponentVector(m_componentReflector, componentType, componentSize);
		}
	}
	Dev::FatalAssert(componentVector.GetComponentType() == componentType,
		"Mismatch between component vector type and the key it is stored at.");

	if (!m_componentReflector.TryMakeComponent(componentInfo, componentID, componentVector))
	{
		Dev::LogWarning("Failed to create component of type [%s].", componentInfo.GetTypeName());
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

	// Flag the ECS group vectors for recalculation.
	m_ecsGroupVectorsNeedRecalculation = true;

	// If we can transmit state, track the removed component.
	if (m_transmissionBuffers != nullptr)
	{
		m_transmissionBuffers->m_componentsRemovedSinceLastTransmission.Add(id);
	}
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
							[&](const ComponentID& componentID)
							{
								return componentID.GetType().GetTypeHash() == typeHash;
							});
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
		// If the group has only one system, run it directly on this thread.
		if (concurrentGroup.m_systems.Size() == 1)
		{
			concurrentGroup.m_systems.Front().m_updateFunction(*this, concurrentGroup.m_systems.Front());
		}
		else
		{
			std::for_each(std::execution::par, concurrentGroup.m_systems.begin(), concurrentGroup.m_systems.end(),
				[&](RegisteredSystem& registeredSystem)
				{
					registeredSystem.m_updateFunction(*this, registeredSystem);
				});
		}

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
