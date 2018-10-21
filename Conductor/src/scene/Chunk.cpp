#include <scene/Chunk.h>

#include <ecs/Component.h>
#include <ecs/Entity.h>
#include <ecs/EntityInfoManager.h>
#include <ecs/EntityManager.h>
#include <file/FullFileReader.h>
#include <file/JSONReader.h>

namespace Internal_Chunk
{
const Util::StringHash k_entitiesHash = Util::CalcHash("entities");
const Util::StringHash k_idHash = Util::CalcHash("id");
const Util::StringHash k_infoHash = Util::CalcHash("info");
const Util::StringHash k_componentsHash = Util::CalcHash("components");
}

namespace Scene
{
JSON::JSONObject Chunk::SaveInPlayChunk(const ChunkID chunkID, const ECS::EntityManager& entityManager,
	const Collection::Vector<const ECS::Entity*>& entitiesInChunk)
{
	// Save the entities in the chunk.
	auto serializedEntities = Mem::MakeUnique<JSON::JSONArray>();

	for (const auto& entity : entitiesInChunk)
	{
		auto serializedEntity = Mem::MakeUnique<JSON::JSONObject>();
		{
			auto serializedID = Mem::MakeUnique<JSON::JSONNumber>();
			serializedID->m_number = entity->GetID().GetUniqueID();
			serializedEntity->Emplace("id", std::move(serializedID));
		}
		{
			auto serializedInfoName = Mem::MakeUnique<JSON::JSONString>();
			serializedInfoName->m_hash = entity->GetInfoNameHash();
			serializedInfoName->m_string = Util::ReverseHash(entity->GetInfoNameHash());
			serializedEntity->Emplace("info", std::move(serializedInfoName));
		}
		{
			auto serializedComponents = Mem::MakeUnique<JSON::JSONObject>();

			for (const auto& componentID : entity->GetComponentIDs())
			{
				const ECS::Component& component = *entityManager.FindComponent(componentID);
				JSON::JSONObject serializedComponent = component.Save();
				const std::string componentTypeName = Util::ReverseHash(component.m_id.GetType().GetTypeHash());
				serializedComponents->Emplace(componentTypeName,
					Mem::MakeUnique<JSON::JSONObject>(std::move(serializedComponent)));
			}

			serializedEntity->Emplace("components", std::move(serializedComponents));
		}

		serializedEntities->Add(std::move(serializedEntity));
	}

	JSON::JSONObject serializedChunk;
	serializedChunk.Emplace("entities", std::move(serializedEntities));
	return serializedChunk;
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

	Mem::UniquePtr<JSON::JSONObject> serializedChunk = File::ReadJSONFile(rawChunk.c_str());
	if (serializedChunk == nullptr)
	{
		return outChunk;
	}

	// Load the entities in the chunk.
	JSON::JSONArray* const serializedEntities = serializedChunk->FindArray(k_entitiesHash);
	if (serializedEntities == nullptr)
	{
		return outChunk;
	}

	for (auto& rawSerializedEntity : *serializedEntities)
	{
		if (rawSerializedEntity->GetType() != JSON::ValueType::Object)
		{
			continue;
		}
		auto& serializedEntity = *static_cast<JSON::JSONObject*>(rawSerializedEntity.Get());

		const auto serializedID = serializedEntity.FindNumber(k_idHash);
		const auto serializedInfoName = serializedEntity.FindString(k_infoHash);
		auto serializedComponents = serializedEntity.FindObject(k_componentsHash);
		if (serializedID == nullptr || serializedInfoName == nullptr || serializedComponents == nullptr)
		{
			continue;
		}

		SerializedEntity& entityDatum = outChunk.m_entityData.Emplace();
		entityDatum.m_entityID = ECS::EntityID(static_cast<uint32_t>(serializedID->m_number));
		entityDatum.m_entityInfoNameHash = serializedInfoName->m_hash;

		for (auto& entry : *serializedComponents)
		{
			const std::string& componentTypeName = entry.first;
			Mem::UniquePtr<JSON::JSONValue>& rawSerializedComponent = entry.second;

			if (rawSerializedComponent->GetType() != JSON::ValueType::Object)
			{
				continue;
			}
			JSON::JSONObject& serializedComponent = *static_cast<JSON::JSONObject*>(rawSerializedComponent.Get());
			// TODO don't recalculate the hash
			entityDatum.m_serializedComponents.Emplace(Util::CalcHash(componentTypeName), std::move(serializedComponent));
		}
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
		for (const auto& serializedComponentEntry : entityDatum.m_serializedComponents)
		{
			const Util::StringHash typeHash = serializedComponentEntry.first;
			const ECS::ComponentID componentID = entity->FindComponentID(ECS::ComponentType(typeHash));
			if (componentID == ECS::ComponentID())
			{
				Dev::LogWarning("Failed to find a component with type [%s] in entity with ID [%u].",
					Util::ReverseHash(typeHash), entityDatum.m_entityID.GetUniqueID());
				continue;
			}

			ECS::Component& component = *entityManager.FindComponent(componentID);
			component.Load(serializedComponentEntry.second);
		}
	}
}
}
