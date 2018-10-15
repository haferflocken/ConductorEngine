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
	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const;
};
}
