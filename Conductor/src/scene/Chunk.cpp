#include <scene/Chunk.h>

#include <ecs/Component.h>
#include <ecs/Entity.h>
#include <ecs/EntityInfoManager.h>
#include <ecs/EntityManager.h>
#include <file/FullFileReader.h>
#include <mem/Serialize.h>

namespace Internal_Chunk
{
const Util::StringHash k_entitiesHash = Util::CalcHash("entities");
const Util::StringHash k_idHash = Util::CalcHash("id");
const Util::StringHash k_infoHash = Util::CalcHash("info");
const Util::StringHash k_componentsHash = Util::CalcHash("components");

template <typename T>
void SerializeBytes(const T& e, Collection::Vector<char>& outBytes)
{
	const char* const eBytes = reinterpret_cast<const char*>(&e);
	outBytes.AddAll({ eBytes, sizeof(T) });
}

void SerializeString(const char* const str, Collection::Vector<char>& outBytes)
{
	size_t strLength = 0;
	for (const char* c = str; *c != '\0'; ++c)
	{
		++strLength;
	}
	const uint16_t shortStrLength = static_cast<uint16_t>(strLength);
	SerializeBytes(shortStrLength, outBytes);
	outBytes.AddAll({ str, strLength });
}
}

namespace Scene
{
Collection::Vector<uint8_t> Chunk::SaveInPlayChunk(const ChunkID chunkID, const ECS::EntityManager& entityManager,
	const Collection::Vector<const ECS::Entity*>& entitiesInChunk)
{
	using namespace Internal_Chunk;

	// Save the entities in the chunk.
	Collection::Vector<uint8_t> chunkBytes;

	const uint32_t numEntities = entitiesInChunk.Size();
	Mem::Serialize(numEntities, chunkBytes);

	for (const auto& entity : entitiesInChunk)
	{
		const uint32_t entityID = entity->GetID().GetUniqueID();
		Mem::Serialize(entityID, chunkBytes);

		const char* const infoName = Util::ReverseHash(entity->GetInfoNameHash());
		Mem::Serialize(infoName, chunkBytes);

		const uint16_t numComponents = static_cast<uint16_t>(entity->GetComponentIDs().Size());
		Mem::Serialize(numComponents, chunkBytes);

		for (const auto& componentID : entity->GetComponentIDs())
		{
			const char* const componentTypeName = Util::ReverseHash(componentID.GetType().GetTypeHash());
			Mem::Serialize(componentTypeName, chunkBytes);

			const uint64_t componentUniqueID = componentID.GetUniqueID();
			Mem::Serialize(componentUniqueID, chunkBytes);

			// Serialize a placeholder for the length of the component bytes.
			const uint32_t numComponentBytesIndex = chunkBytes.Size();
			Mem::Serialize(uint16_t(0), chunkBytes);

			// Serialize the component to bytes and store its length before it.
			const uint32_t componentBytesIndex = chunkBytes.Size();

			const ECS::Component& component = *entityManager.FindComponent(componentID);
			component.Save(chunkBytes);

			const uint32_t numComponentBytes = chunkBytes.Size() - componentBytesIndex;

			chunkBytes[numComponentBytesIndex] = static_cast<uint8_t>(numComponentBytes >> 8);
			chunkBytes[numComponentBytesIndex + 1] = static_cast<uint8_t>(numComponentBytes);
		}
	}

	return chunkBytes;
}

Chunk Chunk::LoadChunkForPlay(const File::Path& sourcePath, const File::Path& userPath,
	const std::string& chunkFileName)
{
	using namespace Internal_Chunk;
	Chunk outChunk;

	// If the chunk exists at the user path, load it. Otherwise load it from the source path.
	std::string rawChunk = File::ReadFullTextFile(userPath / chunkFileName);
	if (rawChunk.empty())
	{
		rawChunk = File::ReadFullTextFile(sourcePath / chunkFileName);
	}

	const uint8_t* iter = reinterpret_cast<const uint8_t*>(rawChunk.data());
	const uint8_t* const iterEnd = iter + rawChunk.size();
	
	// Load the entities.
	const Collection::Pair<uint32_t, bool> maybeNumEntities = Mem::DeserializeUi32(iter, iterEnd);
	if (!maybeNumEntities.second)
	{
		return outChunk;
	}
	const uint32_t numEntities = maybeNumEntities.first;

	for (size_t i = 0, iEnd = numEntities; i < iEnd; ++i)
	{
		// Create the EntityDatum.
		const Collection::Pair<uint32_t, bool> maybeEntityID = Mem::DeserializeUi32(iter, iterEnd);
		if (!maybeEntityID.second)
		{
			return outChunk;
		}
		const uint32_t entityID = maybeEntityID.first;

		char entityInfoNameBuffer[128];
		if (!Mem::DeserializeString(iter, iterEnd, entityInfoNameBuffer))
		{
			return outChunk;
		}
		const Util::StringHash entityInfoNameHash = Util::CalcHash(entityInfoNameBuffer);

		SerializedEntity entityDatum;
		entityDatum.m_entityID = ECS::EntityID(entityID);
		entityDatum.m_entityInfoNameHash = entityInfoNameHash;

		// Extract the component memory.
		const Collection::Pair<uint16_t, bool> maybeNumComponents = Mem::DeserializeUi16(iter, iterEnd);
		if (!maybeNumComponents.second)
		{
			return outChunk;
		}

		for (size_t j = 0, jEnd = maybeNumComponents.first; j < jEnd; ++j)
		{
			char componentTypeNameBuffer[128];
			if (!Mem::DeserializeString(iter, iterEnd, componentTypeNameBuffer))
			{
				return outChunk;
			}
			const Util::StringHash componentTypeHash = Util::CalcHash(componentTypeNameBuffer);

			const Collection::Pair<uint64_t, bool> maybeComponentID = Mem::DeserializeUi64(iter, iterEnd);
			if (!maybeComponentID.second)
			{
				return outChunk;
			}
			const uint64_t componentID = maybeComponentID.first;
			
			const Collection::Pair<uint16_t, bool> maybeNumComponentBytes = Mem::DeserializeUi16(iter, iterEnd);
			if (!maybeNumComponentBytes.second)
			{
				return outChunk;
			}
			const uint16_t numComponentBytes = maybeNumComponentBytes.first;

			// Early out if we can't copy all of the component's bytes.
			if ((iter + numComponentBytes) > iterEnd)
			{
				return outChunk;
			}

			SerializedComponent& serializedComponent = entityDatum.m_serializedComponents.Emplace();
			serializedComponent.m_componentType = ECS::ComponentType(componentTypeHash);
			serializedComponent.m_componentBytes.AddAll({ iter, numComponentBytes });
			iter += numComponentBytes;
		}

		// Store the EntityDatum once all components are extracted succesfully.
		outChunk.m_entityData.Add(std::move(entityDatum));
	}

	return outChunk;
}

Chunk::Chunk()
{}

Chunk::~Chunk()
{}

void Chunk::PutChunkEntitiesIntoPlay(
	const ECS::EntityInfoManager& entityInfoManager,
	ECS::EntityManager& entityManager)
{
	for (auto& entityDatum : m_entityData)
	{
		// Serialized entities may have invalid data; skip them when that happens.
		const ECS::EntityInfo* const info = entityInfoManager.FindEntityInfo(entityDatum.m_entityInfoNameHash);
		if (info == nullptr)
		{
			Dev::LogWarning("Failed to find entity info for serialized entity with ID [%u].",
				entityDatum.m_entityID.GetUniqueID());
			continue;
		}

		// Create the entity if it does not exist.
		ECS::Entity* entity = entityManager.FindEntity(entityDatum.m_entityID);
		if (entity == nullptr)
		{
			// TODO create with specific ID?
			entity = &entityManager.CreateEntity(*info);
		}
		// If the entity exists, ensure it has the correct components.
		else
		{
			entityManager.SetInfoForEntity(*info, *entity);
		}
		
		// Apply the serialized component data to the entity.
		for (const auto& serializedComponent : entityDatum.m_serializedComponents)
		{
			const ECS::ComponentID componentID = entity->FindComponentID(serializedComponent.m_componentType);
			if (componentID == ECS::ComponentID())
			{
				Dev::LogWarning("Failed to find a component with type [%s] in entity with ID [%u].",
					Util::ReverseHash(serializedComponent.m_componentType.GetTypeHash()),
					entityDatum.m_entityID.GetUniqueID());
				continue;
			}

			ECS::Component& component = *entityManager.FindComponent(componentID);
			component.Load(serializedComponent.m_componentBytes);
		}
	}
}
}
