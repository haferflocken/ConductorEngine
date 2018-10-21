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

		const uint32_t chunkX = static_cast<uint32_t>(position.x) >> Chunk::k_lgSideLength;
		const uint32_t chunkY = static_cast<uint32_t>(position.y) >> Chunk::k_lgSideLength;
		const uint32_t chunkZ = static_cast<uint32_t>(position.z) >> Chunk::k_lgSideLength;

		bool found = false;
		for (auto& anchorChunkID : m_anchorChunkIDs)
		{
			if (anchorChunkID.GetX() == chunkX && anchorChunkID.GetY() == chunkY
				&& anchorChunkID.GetZ() == chunkZ)
			{
				anchorChunkID = ChunkID(chunkX, chunkY, chunkZ, k_extraMarker);
				found = true;
			}
		}

		if (!found)
		{
			m_anchorChunkIDs.Emplace(chunkX, chunkY, chunkZ, k_extraMarker);
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
