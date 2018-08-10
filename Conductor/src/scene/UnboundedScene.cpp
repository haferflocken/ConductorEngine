#include <scene/UnboundedScene.h>

#include <ecs/EntityManager.h>
#include <json/JSONPrintVisitor.h>

#include <fstream>
#include <random>

namespace Internal_UnboundedScene
{
std::string MakeChunkIDString(const Scene::ChunkID& chunkID)
{
	return '(' + std::to_string(chunkID.GetX()) + ")(" + std::to_string(chunkID.GetY())
		+ ")(" + std::to_string(chunkID.GetZ()) + ')';
}
}

namespace Scene
{
UnboundedScene::UnboundedScene(const ECS::EntityInfoManager& entityInfoManager)
	: m_entityInfoManager(entityInfoManager)
	, m_spatialHashMap(ChunkHashFunctor(), 12)
	, m_chunksInPlay()
	, m_transitionChunksToRefCounts()
{}

void UnboundedScene::BringChunkIntoPlay(const ChunkID chunkID)
{
	if (m_chunksInPlay.IndexOf(chunkID) != m_chunksInPlay.sk_InvalidIndex)
	{
		return;
	}
	m_chunksInPlay.Add(chunkID);

	// Begin asynchronously loading the chunk.
	m_chunkLoadingFutures.Add(std::async(std::launch::async, Chunk::LoadChunkForPlay,
		m_filePath / Internal_UnboundedScene::MakeChunkIDString(chunkID)));

	// If the chunk is in the transition zone, remove it.
	m_transitionChunksToRefCounts.TryRemove(chunkID);

	// Add the chunks adjacent to the chunk to the transition zone if they are not in play.
	// If any are already there, just increment their refernce count.
	for (int32_t z = -1; z <= 1; ++z)
	{
		for (int32_t y = -1; y <= 1; ++y)
		{
			for (int32_t x = -1; x <= 1; ++x)
			{
				const ChunkID adjacentChunkID{ chunkID.GetX() + x, chunkID.GetY() + y, chunkID.GetZ() + z };
				if (m_chunksInPlay.IndexOf(adjacentChunkID) == m_chunksInPlay.sk_InvalidIndex)
				{
					m_transitionChunksToRefCounts[adjacentChunkID] += ChunkRefCount(1);
				}
			}
		}
	}
}

void UnboundedScene::RemoveChunkFromPlay(const ChunkID chunkID)
{
	const size_t chunkIndex = m_chunksInPlay.IndexOf(chunkID);
	if (chunkIndex == m_chunksInPlay.sk_InvalidIndex)
	{
		return;
	}
	m_chunksInPlay.SwapWithAndRemoveLast(chunkIndex);
	m_chunksPendingRemoval.Add(chunkID);

	// Decrement the reference counts of all adjacent transition chunks.
	for (int32_t z = -1; z <= 1; ++z)
	{
		for (int32_t y = -1; y <= 1; ++y)
		{
			for (int32_t x = -1; x <= 1; ++x)
			{
				const ChunkID adjacentChunkID{ chunkID.GetX() + x, chunkID.GetY() + y, chunkID.GetZ() + z };
				auto* const entry = m_transitionChunksToRefCounts.Find(adjacentChunkID);
				Dev::FatalAssert(entry != nullptr,
					"Transition chunk reference counts are not being maintained correctly.");

				entry->second -= ChunkRefCount(1);
			}
		}
	}
}

void UnboundedScene::Update(ECS::EntityManager& entityManager,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void()>>& deferredFunctions)
{
	// Update the hash map with the location of all entities.
	m_spatialHashMap.Clear();
	for (const auto& ecsGroup : ecsGroups)
	{
		const ECS::Entity& entity = ecsGroup.Get<ECS::Entity>(entityManager);
		const auto& sceneTransformComponent = ecsGroup.Get<const SceneTransformComponent>(entityManager);
		const Math::Vector3& position = sceneTransformComponent.m_matrix.GetTranslation();
		m_spatialHashMap[position] = &entity;
	}

	// Apply all pending chunk changes.
	// This invalidates ecsGroups by adding and removing entities as well as changing the components on them.
	FlushPendingChunks(entityManager);
}

void UnboundedScene::FlushPendingChunks(ECS::EntityManager& entityManager)
{
	// Save and unload any unreferenced transition chunks.
	m_transitionChunksToRefCounts.RemoveAllMatching([&](const Collection::Pair<ChunkID, ChunkRefCount>& entry)
	{
		if (entry.second <= ChunkRefCount(1))
		{
			SaveChunkAndQueueEntitiesForUnload(entityManager, entry.first);
			return true;
		}
		return false;
	});

	// Save and unload any chunks pending removal.
	for (const auto& chunkID : m_chunksPendingRemoval)
	{
		SaveChunkAndQueueEntitiesForUnload(entityManager, chunkID);
	}
	m_chunksPendingRemoval.Clear();

	// Unload all entities from chunks that were saved. This will invalidate pointers in m_spatialHashMap,
	// so it should not be used at any further point of FlushPendingChunks.
	// TODO(scene) partial unloading support for RoPEs
	entityManager.DeleteEntities(m_entitiesPendingUnload.GetConstView());
	m_entitiesPendingUnload.Clear();

	// Synchronize with any chunks that are loading. There is a fixed amount of time that the scene
	// will spend waiting for a chunk before it is deferred to the next frame.
	if (m_chunkLoadingFutures.IsEmpty())
	{
		return;
	}

	constexpr size_t k_syncBudgetMilliseconds = 8;
	constexpr size_t k_syncBudgetMicroseconds = k_syncBudgetMilliseconds * 1000;
	
	const std::chrono::microseconds waitPerFuture{ k_syncBudgetMicroseconds / m_chunkLoadingFutures.Size() };
	for (size_t i = 0; i < m_chunkLoadingFutures.Size();)
	{
		auto& future = m_chunkLoadingFutures[i];

		const std::future_status status = future.wait_for(waitPerFuture);
		Dev::FatalAssert(status != std::future_status::deferred, "Chunk loading should always be asynchronous.");

		if (status == std::future_status::ready)
		{
			Scene::Chunk chunk = future.get();
			chunk.PutChunkEntitiesIntoPlay(m_entityInfoManager, entityManager);

			m_chunkLoadingFutures.SwapWithAndRemoveLast(i);
		}
		else
		{
			++i;
		}
	}
}

void UnboundedScene::SaveChunkAndQueueEntitiesForUnload(ECS::EntityManager& entityManager, const ChunkID chunkID)
{
	// Determine the entities that are in the chunk.
	const Math::Vector3 chunkOrigin = Math::Vector3(
		static_cast<float>(chunkID.GetX()),
		static_cast<float>(chunkID.GetY()),
		static_cast<float>(chunkID.GetZ())) * Chunk::k_sideLengthMeters;

	const Math::Vector3 chunkCenter = chunkOrigin + (Math::Vector3(0.5f, 0.5f, 0.5f) * Chunk::k_sideLengthMeters);
	const Math::Vector3 chunkBound = chunkOrigin + (Math::Vector3(1.0f, 1.0f, 1.0f) * Chunk::k_sideLengthMeters);

	const auto chunkBucketView = m_spatialHashMap.GetBucketView(chunkCenter);

	Collection::Vector<const ECS::Entity*> entitiesInChunk;
	for (size_t i = 0, iEnd = chunkBucketView.m_keys.Size(); i < iEnd; ++i)
	{
		const Math::Vector3& position = chunkBucketView.m_keys[i];
		const ECS::Entity* const entity = chunkBucketView.m_values[i];

		if (position.x >= chunkOrigin.x && position.y >= chunkOrigin.y && position.z >= chunkOrigin.z
			&& position.x < chunkBound.x && position.y < chunkBound.y && position.z < chunkBound.z)
		{
			entitiesInChunk.Add(entity);
		}
	}

	// Save the chunk to its file.
	const JSON::JSONObject serializedChunk = Chunk::SaveInPlayChunk(chunkID, entityManager, entitiesInChunk);

	std::ofstream fileOutput;
	fileOutput.open(m_filePath / Internal_UnboundedScene::MakeChunkIDString(chunkID));

	JSON::PrintVisitor printVisitor{ fileOutput };
	serializedChunk.Accept(&printVisitor);

	fileOutput.flush();
	fileOutput.close();

	// Add the entities in the chunk to the list of entities to unload. This is deferred
	// (as opposed to unloading entities immediately) because removing entities from the EntityManager
	// will invalidate the pointers in m_spatialHashMap.
	for (const auto& entity : entitiesInChunk)
	{
		m_entitiesPendingUnload.Add(entity);
	}
}

UnboundedScene::ChunkHashFunctor::ChunkHashFunctor()
	: m_a()
	, m_b()
	, m_c()
	, m_d()
{
	Rehash();
}

uint64_t UnboundedScene::ChunkHashFunctor::Hash(const Math::Vector3& position) const
{
	constexpr int64_t k_bucketSideLengthMetersShift = Chunk::k_lgSideLength;
	const int64_t bucketX = static_cast<int64_t>(position.x) >> k_bucketSideLengthMetersShift;
	const int64_t bucketY = static_cast<int64_t>(position.y) >> k_bucketSideLengthMetersShift;
	const int64_t bucketZ = static_cast<int64_t>(position.z) >> k_bucketSideLengthMetersShift;

	return static_cast<uint64_t>((bucketX * m_a) + (bucketY * m_b) + (bucketZ * m_c) + m_d);
}

void UnboundedScene::ChunkHashFunctor::Rehash()
{
	std::random_device device;
	std::mt19937_64 generator{ device() };
	std::uniform_int_distribution<int64_t> random;
	m_a = random(generator) | 1;
	m_b = random(generator) | 1;
	m_c = random(generator) | 1;
	m_d = random(generator) | 1;
}
}
