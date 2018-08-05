#include <scene/Chunk.h>

#include <ecs/Component.h>
#include <ecs/Entity.h>
#include <ecs/EntityInfoManager.h>
#include <ecs/EntityManager.h>
#include <file/JSONReader.h>

namespace Internal_Chunk
{
const Util::StringHash k_typeKeyHash = Util::CalcHash("type");
}

namespace Scene
{
JSON::JSONObject Chunk::SaveInPlayChunk(const ChunkID chunkID, const ECS::EntityManager& entityManager)
{
	// TODO save the entities in the chunk
	return JSON::JSONObject();
}

Chunk Chunk::LoadChunkForPlay(const File::Path& chunkFilePath)
{
	Mem::UniquePtr<JSON::JSONObject> serializedChunk = File::ReadJSONFile(chunkFilePath);

	// TODO load the entities in the chunk

	return Chunk();
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
			const JSON::JSONString* const typeString = serializedComponent.FindString(Internal_Chunk::k_typeKeyHash);
			if (typeString == nullptr)
			{
				Dev::LogWarning("Failed to find a component type string for entity with ID [%u].",
					entityDatum.m_entityID.GetUniqueID());
				continue;
			}

			const ECS::ComponentID componentID = entity->FindComponentID(typeString->m_hash);
			if (componentID == ECS::ComponentID())
			{
				Dev::LogWarning("Failed to find a component with type [%s] in entity with ID [%u].",
					typeString->m_string.c_str(), entityDatum.m_entityID.GetUniqueID());
				continue;
			}

			ECS::Component& component = *entityManager.FindComponent(componentID);
			component.Load(serializedComponent);
		}
	}
}
}
