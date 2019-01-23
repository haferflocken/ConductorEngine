#pragma once

#include <file/Path.h>
#include <math/Vector3.h>
#include <scene/ChunkID.h>

#include <iosfwd>

namespace ECS
{
class Entity;
class EntityManager;
struct SerializedEntitiesAndComponents;
}

namespace Scene
{
/**
 * A chunk is a discrete portion of a scene that can be saved and loaded from disk.
 * Because a scene only consists of entities, a chunk only consists of entities. A chunk on disk is a serialized
 * representation of the in-play data of entities; out-of-play data is serialized to a global archive.
 * Entities in a chunk are simluated within an EntityManager; the Chunk class is just used for saving and loading.
 */

 // The dimensions of a Chunk must always be a power of two.
static constexpr float k_chunkSideLengthMeters = 64.0f;
static constexpr uint32_t k_lgChunkSideLength = 6;

static void SaveInPlayChunk(const ChunkID chunkID,
	const ECS::EntityManager& entityManager,
	const Collection::Vector<const ECS::Entity*>& rootEntitiesInChunk,
	std::ofstream& fileOutput);
static ECS::SerializedEntitiesAndComponents LoadChunkForPlay(const File::Path& sourcePath,
	const File::Path& userPath,
	const std::string& chunkFileName);

static Math::Vector3 CalcChunkOrigin(const ChunkID chunkID);

static void CalcChunkCoords(const ChunkID chunkID,
	Math::Vector3& outOrigin, Math::Vector3& outCenter, Math::Vector3& outMax);
}
