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
	// TODO
}
}
