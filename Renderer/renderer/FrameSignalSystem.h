#pragma once

#include <ecs/System.h>
#include <scene/SceneTransformComponent.h>
#include <scene/SceneTransformComponentInfo.h>

namespace Renderer
{
/**
 * Sends a message from the client thread to the render thread when a frame can be processed.
 */
class FrameSignalSystem final : public ECS::SystemTempl<Util::TypeList<Scene::SceneTransformComponent>, Util::TypeList<>>
{
public:
	void Update(ECS::EntityManager& entityManager,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void()>>& deferredFunctions) const;
};
}