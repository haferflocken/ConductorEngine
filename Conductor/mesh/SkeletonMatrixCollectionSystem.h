#pragma once

#include <ecs/System.h>

#include <mesh/MeshComponent.h>
#include <mesh/SkeletonRootComponent.h>

namespace Mesh
{
/**
 * The SkeletonMatrixCollectionSystem takes the world matricies of the bones of a entity's skeleton and stores them in
 * its MeshComponent.
 */
class SkeletonMatrixCollectionSystem final : public ECS::SystemTempl<
	Util::TypeList<SkeletonRootComponent>,
	Util::TypeList<MeshComponent>>
{
public:
	SkeletonMatrixCollectionSystem() = default;
	virtual ~SkeletonMatrixCollectionSystem() = default;

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const;
};
}
