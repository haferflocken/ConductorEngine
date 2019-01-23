#include <scene/Chunk.h>

#include <ecs/ComponentReflector.h>
#include <ecs/Entity.h>
#include <ecs/EntityManager.h>
#include <ecs/SerializedEntitiesAndComponents.h>
#include <file/FullFileReader.h>
#include <mem/SerializeLittleEndian.h>

#include <fstream>

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

	// Serialize the entity views and component views.
	Collection::Vector<uint8_t> viewBytes;

	const uint32_t numComponentTypes = serialization.m_componentViews.Size();
	Mem::LittleEndian::Serialize(numComponentTypes, viewBytes);

	for (const auto& entry : serialization.m_componentViews)
	{
		const char* const componentTypeName = Util::ReverseHash(entry.first.GetTypeHash());
		Mem::LittleEndian::Serialize(componentTypeName, viewBytes);

		const auto& componentViews = entry.second;

		const uint32_t numComponentViews = componentViews.Size();
		Mem::LittleEndian::Serialize(numComponentViews, viewBytes);

		const uint32_t componentViewsIndex = viewBytes.Size();
		const uint32_t sizeOfComponentViews = numComponentViews * sizeof(ECS::SerializedByteView);
		viewBytes.Resize(viewBytes.Size() + sizeOfComponentViews);
		memcpy(&viewBytes[componentViewsIndex], &componentViews.Front(), sizeOfComponentViews);
	}

	const uint32_t numEntityViews = serialization.m_entityViews.Size();
	Mem::LittleEndian::Serialize(numEntityViews, viewBytes);

	const uint32_t entityViewsIndex = viewBytes.Size();
	const uint32_t sizeOfEntityViews = numEntityViews * sizeof(ECS::SerializedByteView);
	viewBytes.Resize(viewBytes.Size() + sizeOfEntityViews);
	memcpy(&viewBytes[entityViewsIndex], &serialization.m_entityViews.Front(), sizeOfEntityViews);

	// Write the serialized views to the output.
	fileOutput.write(reinterpret_cast<const char*>(&viewBytes.Front()), viewBytes.Size());

	// Write the serialized entities and components to the output.
	fileOutput.write(reinterpret_cast<const char*>(&serialization.m_bytes.Front()), serialization.m_bytes.Size());
}

ECS::SerializedEntitiesAndComponents Scene::LoadChunkForPlay(const File::Path& sourcePath,
	const File::Path& userPath,
	const std::string& chunkFileName)
{
	ECS::SerializedEntitiesAndComponents outChunk;

	// If the chunk exists at the user path, load it. Otherwise load it from the source path.
	std::string rawChunk = File::ReadFullTextFile(userPath / chunkFileName);
	if (rawChunk.empty())
	{
		rawChunk = File::ReadFullTextFile(sourcePath / chunkFileName);
		if (rawChunk.empty())
		{
			return outChunk;
		}
	}

	const uint8_t* iter = reinterpret_cast<const uint8_t*>(rawChunk.data());
	const uint8_t* const iterEnd = iter + rawChunk.size();

	// TODO(info) load the serialized views, then load the serialized entities and components

	return outChunk;
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
