#pragma once

#include <collection/HashMap.h>
#include <collection/Vector.h>
#include <collection/VectorMap.h>

#include <ecs/Entity.h>
#include <ecs/SerializedEntitiesAndComponents.h>
#include <ecs/System.h>

#include <file/Path.h>
#include <math/Vector3.h>
#include <scene/Chunk.h>
#include <scene/ChunkID.h>
#include <scene/SceneSaveComponent.h>
#include <scene/SceneTransformComponent.h>
#include <unit/UnitTempl.h>

#include <future>

namespace ECS
{
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
 * Entity hierarchies are always considered to be in the chunk that their root entity is in.
 * 
 * Each chunk in an unbounded scene is either in-play or out-of-play. In-play chunks run a full simulation of their
 * entities. Out-of-play chunks run an extremely simplified simulation. Chunks on the boundary of in-play and
 * out-of-play are called "transition" chunks which buffer the scene from in/out of play oscillations.
 *
 * UnboundedScenes are ECS::Systems and should not run concurrently with other systems because they add entities
 * and remove entities as they run.
 */
class UnboundedScene final : public ECS::SystemTempl<
	Util::TypeList<Scene::SceneTransformComponent, Scene::SceneSaveComponent>,
	Util::TypeList<ECS::Entity>>
{
public:
	UnboundedScene(const File::Path& sourcePath, const File::Path& userPath);
	virtual ~UnboundedScene() {}

	virtual void NotifyOfShutdown(ECS::EntityManager& entityManager) override;

	void BringChunkIntoPlay(const ChunkID chunkID);
	void RemoveChunkFromPlay(const ChunkID chunkID);

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

private:
	void FlushPendingChunks(ECS::EntityManager& entityManager);
	void SaveChunkAndQueueEntitiesForUnload(ECS::EntityManager& entityManager, const ChunkID chunkID);

	// The directory the scene's chunks will be loaded from the first time they are loaded.
	File::Path m_sourcePath;
	// The directory the scene's chunks will be stored in.
	File::Path m_userPath;

	// A spatial hash that buckets entities by the chunk they are in.
	class ChunkHashFunctor
	{
		int64_t m_a;
		int64_t m_b;
		int64_t m_c;
		int64_t m_d;

	public:
		ChunkHashFunctor();

		uint64_t Hash(const Math::Vector3& position) const;
		void Rehash();
	};
	Collection::HashMap<Math::Vector3, const ECS::Entity*, ChunkHashFunctor> m_spatialHashMap;

	Collection::Vector<ChunkID> m_chunksInPlay;
	Collection::Vector<ChunkID> m_chunksPendingRemoval;
	Collection::Vector<std::future<ECS::SerializedEntitiesAndComponents>> m_chunkLoadingFutures;

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

	Collection::Vector<ECS::EntityID> m_entitiesPendingUnload;
};
}
