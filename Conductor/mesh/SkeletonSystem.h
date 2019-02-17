#pragma once

#include <ecs/EntityID.h>
#include <ecs/System.h>

#include <mesh/MeshComponent.h>
#include <mesh/SkeletonRootComponent.h>

namespace Mesh
{
/**
 * The SkeletonSystem instantiates the bone child entities of entities with a SkeletonRootComponent and a MeshComponent.
 */
class SkeletonSystem final : public ECS::SystemTempl<
	Util::TypeList<MeshComponent>,
	Util::TypeList<ECS::Entity, SkeletonRootComponent>,
	ECS::SystemBindingType::Extended>
{
public:
	SkeletonSystem() = default;
	virtual ~SkeletonSystem() = default;

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);
	
	void NotifyOfEntityAdded(const ECS::EntityID id, const ECSGroupType& group);
	void NotifyOfEntityRemoved(const ECS::EntityID id, const ECSGroupType& group);

private:
	// Tracks the entities that have been added since the last update.
	Collection::Vector<const MeshComponent*> m_newEntityMeshComponents;

	// Tracks the entities that still need their skeleton instantiated.
	Collection::Vector<const MeshComponent*> m_meshComponentsOfEntitiesToProcess;
};
}
