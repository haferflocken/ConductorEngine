#pragma once

#include <ecs/EntityID.h>
#include <file/Path.h>
#include <json/JSONTypes.h>
#include <math/Vector3.h>
#include <scene/ChunkID.h>
#include <util/StringHash.h>

namespace ECS
{
class Entity;
class EntityInfoManager;
class EntityManager;
}

namespace Scene
{
/**
 * A chunk is a discrete portion of a scene that can be saved and loaded from disk.
 * Because a scene only consists of entities, a chunk only consists of entities. A chunk on disk is a serialized
 * representation of the in-play data of entities; out-of-play data is serialized to a global archive.
 * Entities in a chunk are simluated within an EntityManager; the Chunk class is just used for saving and loading.
 */
class Chunk final
{
public:
	// The dimensions of a Chunk must always be a power of two.
	static constexpr float k_sideLengthMeters = 64.0f;
	static constexpr uint32_t k_lgSideLength = 6;

	// TODO Long term, I don't think text serialization will be fast enough for chunk save & load.
	static JSON::JSONObject SaveInPlayChunk(const ChunkID chunkID, const ECS::EntityManager& entityManager,
		const Collection::Vector<const ECS::Entity*>& entitiesInChunk);
	static Chunk LoadChunkForPlay(const File::Path& chunkFilePath);

	Chunk();

	Chunk(const Chunk&) = delete;
	Chunk& operator=(const Chunk&) = delete;

	Chunk(Chunk&& other) = default;
	Chunk& operator=(Chunk&& rhs) = default;

	~Chunk();

	void PutChunkEntitiesIntoPlay(const ECS::EntityInfoManager& entityInfoManager, ECS::EntityManager& entityManager);

private:
	struct SerializedEntity final
	{
		SerializedEntity() = default;

		SerializedEntity(const SerializedEntity&) = delete;
		SerializedEntity& operator=(const SerializedEntity&) = delete;

		SerializedEntity(SerializedEntity&& other) = default;
		SerializedEntity& operator=(SerializedEntity&& rhs) = default;

		ECS::EntityID m_entityID{};
		Util::StringHash m_entityInfoNameHash{};
		Collection::Vector<JSON::JSONObject> m_serializedComponents{};
	};
	Collection::Vector<SerializedEntity> m_entityData{};
};
}
