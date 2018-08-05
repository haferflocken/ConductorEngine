#pragma once

#include <collection/HashMap.h>
#include <collection/Vector.h>
#include <collection/VectorMap.h>
#include <ecs/Entity.h>
#include <ecs/EntityID.h>
#include <ecs/System.h>
#include <file/Path.h>
#include <math/Vector3.h>
#include <scene/Chunk.h>
#include <scene/ChunkID.h>
#include <unit/UnitTempl.h>

#include <future>

namespace ECS
{
class EntityInfoManager;
class EntityManager;
}

namespace Scene
{
/**
 * A scene is a space in which entities exist. An UnboundedScene is a scene that can extend to the bounds of its
 * coordinate system and which does not need to be entirely loaded into memory at all times.
 * 
 * UnboundedScenes are aggreates of Chunks: discrete portions of the scene that can be saved to and loaded from disk.
 * UnboundedScenes are not responsible for deciding what chunks are loaded: they are only responsible for saving and
 *   and loading chunks and updating their EntityManager appropriately.
 * 
 * Each chunk in an unbounded scene is either in-play or out-of-play. In-play chunks run a full simulation of their
 * entities. Out-of-play chunks run an extremely simplified simulation. Chunks on the boundary of in-play and
 * out-of-play are called "transition" chunks which buffer the scene from in/out of play oscillations.
 */
class UnboundedScene final : public ECS::SystemTempl<Util::TypeList<>, Util::TypeList<ECS::Entity>>
{
public:
	explicit UnboundedScene(const ECS::EntityInfoManager& entityInfoManager);

	void BringChunkIntoPlay(const ChunkID chunkID);
	void RemoveChunkFromPlay(const ChunkID chunkID);

	void Update(ECS::EntityManager& entityManager,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void()>>& deferredFunctions);

private:
	void FlushPendingChunks(ECS::EntityManager& entityManager);
	void SaveAndUnloadChunk(ECS::EntityManager& entityManager, const ChunkID chunkID);

	const ECS::EntityInfoManager& m_entityInfoManager;
	File::Path m_filePath;

	class PositionHashFunctor
	{
		int64_t m_a;
		int64_t m_b;
		int64_t m_c;
		int64_t m_d;

	public:
		PositionHashFunctor();

		uint64_t Hash(const Math::Vector3& position) const;
		void Rehash();
	};
	Collection::HashMap<Math::Vector3, ECS::EntityID, PositionHashFunctor> m_spatialHashMap;

	Collection::Vector<ChunkID> m_chunksInPlay;
	Collection::Vector<ChunkID> m_chunksPendingRemoval;
	Collection::Vector<std::future<Chunk>> m_chunkLoadingFutures;

	struct ChunkRefCount : public Unit::UnitTempl<ChunkRefCount, uint8_t>
	{
		explicit constexpr ChunkRefCount()
			: BaseType(0)
		{}

		explicit constexpr ChunkRefCount(uint8_t v)
			: BaseType(v)
		{}
	};
	Collection::VectorMap<ChunkID, ChunkRefCount> m_transitionChunksToRefCounts;
};
}
