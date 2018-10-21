#pragma once

#include <ecs/System.h>

#include <scene/AnchorComponent.h>
#include <scene/AnchorComponentInfo.h>
#include <scene/ChunkID.h>
#include <scene/SceneTransformComponent.h>
#include <scene/SceneTransformComponentInfo.h>

namespace Scene
{
class UnboundedScene;

/**
 * The SceneAnchorSystem loads chunks into an UnboundedScene near entities with an AnchorComponent,
 * and unloads chunks that get too far away from those entities.
 */
class SceneAnchorSystem final : public ECS::SystemTempl<
	Util::TypeList<SceneTransformComponent, AnchorComponent>,
	Util::TypeList<>>
{
public:
	explicit SceneAnchorSystem(UnboundedScene& scene);
	virtual ~SceneAnchorSystem() {}

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

private:
	UnboundedScene& m_scene;
	Collection::Vector<ChunkID> m_anchorChunkIDs;
};
}
