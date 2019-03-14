#pragma once

#include <ecs/System.h>
#include <mesh/MeshComponent.h>

namespace Renderer::Debug
{
/**
 * Draws the skeletons of entities.
 */
class SkeletonDebugRenderSystem final : public ECS::SystemTempl<
	Util::TypeList<Mesh::MeshComponent>,
	Util::TypeList<>>
{
public:
	SkeletonDebugRenderSystem() = default;

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const;
};
}
