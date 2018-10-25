#include <scene/SceneAnchorSystem.h>

#include <scene/UnboundedScene.h>

namespace Scene
{
SceneAnchorSystem::SceneAnchorSystem(UnboundedScene& scene)
	: m_scene(scene)
{}

void SceneAnchorSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	// Clear the extra bits of the anchor chunk IDs.
	for (auto& chunkID : m_anchorChunkIDs)
	{
		chunkID = chunkID.GetWithoutExtra();
	}

	// Mark the chunks that need to be anchors.
	static constexpr uint8_t k_extraMarker = 1;
	for (const auto& ecsGroup : ecsGroups)
	{
		const SceneTransformComponent& transformComponent = ecsGroup.Get<const SceneTransformComponent>();
		const Math::Vector3& position = transformComponent.m_matrix.GetTranslation();
		
		const AnchorComponent& anchorComponent = ecsGroup.Get<const AnchorComponent>();

		const int32_t anchoringRadiusInChunks = anchorComponent.m_anchoringRadiusInChunks;

		const float fAnchoringRadiusInChunks = static_cast<float>(anchoringRadiusInChunks) * Chunk::k_sideLengthMeters;
		const float anchoringRadiusSquared = fAnchoringRadiusInChunks * fAnchoringRadiusInChunks;

		const int32_t entityChunkX = static_cast<int32_t>(position.x) >> Chunk::k_lgSideLength;
		const int32_t entityChunkY = static_cast<int32_t>(position.y) >> Chunk::k_lgSideLength;
		const int32_t entityChunkZ = static_cast<int32_t>(position.z) >> Chunk::k_lgSideLength;
		const ChunkID entityChunkID{ entityChunkX, entityChunkY, entityChunkZ };

		const Math::Vector3 entityChunkOrigin = Chunk::CalcChunkOrigin(entityChunkID);

		const int32_t xMin = entityChunkX - anchoringRadiusInChunks;
		const int32_t yMin = entityChunkY - anchoringRadiusInChunks;
		const int32_t zMin = entityChunkZ - anchoringRadiusInChunks;
		const int32_t xMax = entityChunkX + anchoringRadiusInChunks;
		const int32_t yMax = entityChunkY + anchoringRadiusInChunks;
		const int32_t zMax = entityChunkZ + anchoringRadiusInChunks;

		for (int32_t z = zMin; z <= zMax; ++z)
		{
			for (int32_t y = yMin; y <= yMax; ++y)
			{
				for (int32_t x = xMin; x <= xMax; ++x)
				{
					const ChunkID chunkID{ x, y, z };
					const Math::Vector3 chunkOrigin = Chunk::CalcChunkOrigin(chunkID);

					const float distanceSquared = (chunkOrigin - entityChunkOrigin).LengthSquared();
					if (distanceSquared > anchoringRadiusSquared)
					{
						continue;
					}

					bool found = false;
					for (auto& anchorChunkID : m_anchorChunkIDs)
					{
						if (anchorChunkID.GetWithoutExtra() == chunkID)
						{
							anchorChunkID = ChunkID(x, y, z, k_extraMarker);
							found = true;
							break;
						}
					}

					if (!found)
					{
						m_anchorChunkIDs.Emplace(x, y, z, k_extraMarker);
					}
				}
			}
		}
	}

	// Bring all marked chunks into play.
	for (const auto& anchorChunkID : m_anchorChunkIDs)
	{
		if (anchorChunkID.GetExtra() == k_extraMarker)
		{
			m_scene.BringChunkIntoPlay(anchorChunkID.GetWithoutExtra());
		}
	}

	// Remove all unmarked chunks from play and erase them from m_anchorChunkIDs.
	for (const auto& anchorChunkID : m_anchorChunkIDs)
	{
		if (anchorChunkID.GetExtra() != k_extraMarker)
		{
			m_scene.RemoveChunkFromPlay(anchorChunkID.GetWithoutExtra());
		}
	}

	const size_t removeIndex = m_anchorChunkIDs.Partition(
		[](const ChunkID& chunkID) { return chunkID.GetExtra() != k_extraMarker; });
	m_anchorChunkIDs.Remove(removeIndex, m_anchorChunkIDs.Size());
}
}
