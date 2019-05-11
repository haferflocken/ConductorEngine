#include <scene/Chunk.h>

#include <ecs/Entity.h>
#include <ecs/EntityManager.h>
#include <ecs/SerializedEntitiesAndComponents.h>
#include <file/FullFileReader.h>

void Scene::SaveInPlayChunk(const ChunkID chunkID,
	const ECS::EntityManager& entityManager,
	const Collection::Vector<const ECS::Entity*>& rootEntitiesInChunk,
	std::ofstream& fileOutput)
{
	// Gather all the entites in the hierarchy of root entities.
	Collection::Vector<const ECS::Entity*> entitiesToSerialize;
	entitiesToSerialize.AddAll(rootEntitiesInChunk.GetConstView());

	for (size_t i = 0; i < entitiesToSerialize.Size(); ++i)
	{
		const ECS::Entity* const entity = entitiesToSerialize[i];
		AMP_FATAL_ASSERT(entity->GetParent() == nullptr, "Only root entities should be provided to SaveInPlayChunk!");
		entitiesToSerialize.AddAll(entity->GetChildren());
	}

	// Serialize all the entities and their components.
	ECS::SerializedEntitiesAndComponents serialization;
	entityManager.FullySerializeEntitiesAndComponents(entitiesToSerialize.GetView(), serialization);

	// Write the serialization to the file.
	ECS::WriteSerializedEntitiesAndComponentsTo(serialization, fileOutput);
}

ECS::SerializedEntitiesAndComponents Scene::LoadChunkForPlay(const File::Path& sourcePath,
	const File::Path& userPath,
	const std::string& chunkFileName)
{
	// If the chunk exists at the user path, load it. Otherwise load it from the source path.
	std::string rawChunk = File::ReadFullTextFile(userPath / chunkFileName);
	if (rawChunk.empty())
	{
		rawChunk = File::ReadFullTextFile(sourcePath / chunkFileName);
		if (rawChunk.empty())
		{
			return ECS::SerializedEntitiesAndComponents();
		}
	}

	const uint8_t* const bytes = reinterpret_cast<const uint8_t*>(rawChunk.data());

	ECS::SerializedEntitiesAndComponents serialization;
	if (!ECS::TryReadSerializedEntitiesAndComponentsFrom({ bytes, rawChunk.size() }, serialization))
	{
		serialization = ECS::SerializedEntitiesAndComponents();
	}
	return serialization;
}

Math::Vector3 Scene::CalcChunkOrigin(const ChunkID chunkID)
{
	return Math::Vector3(
		static_cast<float>(chunkID.GetX()),
		static_cast<float>(chunkID.GetY()),
		static_cast<float>(chunkID.GetZ())) * k_chunkSideLengthMeters;
}

void Scene::CalcChunkCoords(const ChunkID chunkID,
	Math::Vector3& outOrigin, Math::Vector3& outCenter, Math::Vector3& outMax)
{
	outOrigin = CalcChunkOrigin(chunkID);
	outCenter = outOrigin + (Math::Vector3(0.5f, 0.5f, 0.5f) * k_chunkSideLengthMeters);
	outMax = outOrigin + (Math::Vector3(1.0f, 1.0f, 1.0f) * k_chunkSideLengthMeters);
}
