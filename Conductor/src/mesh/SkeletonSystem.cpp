#include <mesh/SkeletonSystem.h>

#include <ecs/EntityManager.h>
#include <scene/SceneTransformComponent.h>

namespace Mesh
{
namespace Internal_SkeletonSystem
{
void CreateBoneEntities(
	ECS::EntityManager& entityManager,
	ECS::Entity& rootEntity,
	SkeletonRootComponent& skeletonRootComponent,
	const TriangleMesh& mesh)
{
	const Collection::Vector<Math::Matrix4x4>& boneToParentTransforms = mesh.GetBoneToParentTransforms();
	const Collection::Vector<uint16_t>& boneParentIndices = mesh.GetBoneParentIndices();
	AMP_FATAL_ASSERT(boneToParentTransforms.Size() == boneParentIndices.Size(), "Bone count mismatch!");

	const uint32_t numBones = boneToParentTransforms.Size();

	// Create the bone entities and set their relative transforms.
	Collection::Vector<ECS::Entity*> boneEntities;
	boneEntities.Resize(numBones, nullptr);
	for (size_t boneIndex = 0; boneIndex < numBones; ++boneIndex)
	{
		const Math::Matrix4x4& boneToParentTransform = boneToParentTransforms[boneIndex];

		const auto boneComponents = { Scene::SceneTransformComponent::k_type };
		ECS::Entity& boneEntity = entityManager.CreateEntityWithComponents(
			{ boneComponents.begin(), boneComponents.size() });

		auto& boneTransformComponent = *entityManager.FindComponent<Scene::SceneTransformComponent>(boneEntity);
		boneTransformComponent.m_childToParentMatrix = boneToParentTransform;

		boneEntities[boneIndex] = &boneEntity;
	}

	// Set the parent/child relationships of the bone entities.
	for (size_t boneIndex = 0; boneIndex < numBones; ++boneIndex)
	{
		const uint16_t& boneParentIndex = boneParentIndices[boneIndex];
		ECS::Entity& boneEntity = *boneEntities[boneIndex];

		ECS::Entity* const parentEntity =
			(boneParentIndex == TriangleMesh::k_invalidBoneParentIndex) ? &rootEntity : boneEntities[boneParentIndex];

		entityManager.SetParentEntity(boneEntity, parentEntity);
	}
}
}

void SkeletonSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	// Transfer all the new entities to the processing list.
	m_meshComponentsOfEntitiesToProcess.AddAll(m_newEntityMeshComponents.GetConstView());
	m_newEntityMeshComponents.Clear();

	// Process the entities that have finished loading their meshes.
	for (size_t i = 0; i < m_meshComponentsOfEntitiesToProcess.Size(); /* ITERATION CONTROLLED IN LOOP */)
	{
		const MeshComponent* const meshComponent = m_meshComponentsOfEntitiesToProcess[i];
		const TriangleMesh* const mesh = meshComponent->m_meshHandle.TryGetAsset();
		if (mesh == nullptr)
		{
			++i;
			continue;
		}

		for (const auto& group : ecsGroups)
		{
			if (meshComponent != &group.Get<const MeshComponent>())
			{
				continue;
			}

			auto& entity = group.Get<ECS::Entity>();

			auto& skeletonRootComponent = group.Get<SkeletonRootComponent>();
			AMP_ASSERT(skeletonRootComponent.m_boneTransformComponentIDs.IsEmpty(),
				"Double-instantiating the bone hierarchy of a skeleton root entity!");

			// Create an entity for each bone and set the parents accordingly.
			// This must be deferred so it can access the entity manager.
			deferredFunctions.Add(
				[&entity, &skeletonRootComponent, mesh](ECS::EntityManager& entityManager)
				{
					Internal_SkeletonSystem::CreateBoneEntities(entityManager, entity, skeletonRootComponent, *mesh);
				});
			break;
		}

		// Remove the processed component.
		m_meshComponentsOfEntitiesToProcess.SwapWithAndRemoveLast(i);
	}
}

void SkeletonSystem::NotifyOfEntityAdded(const ECS::EntityID id, const ECSGroupType& group)
{
	m_newEntityMeshComponents.Add(&group.Get<const MeshComponent>());
}

void SkeletonSystem::NotifyOfEntityRemoved(const ECS::EntityID id, const ECSGroupType& group)
{
	const MeshComponent* const meshComponent = &group.Get<const MeshComponent>();

	const size_t i = m_newEntityMeshComponents.IndexOf(meshComponent);
	if (i != m_newEntityMeshComponents.sk_InvalidIndex)
	{
		m_newEntityMeshComponents.SwapWithAndRemoveLast(i);
	}
	else
	{
		const size_t j = m_meshComponentsOfEntitiesToProcess.IndexOf(meshComponent);
		if (j != m_meshComponentsOfEntitiesToProcess.sk_InvalidIndex)
		{
			m_meshComponentsOfEntitiesToProcess.SwapWithAndRemoveLast(j);
		}
	}
}
}
