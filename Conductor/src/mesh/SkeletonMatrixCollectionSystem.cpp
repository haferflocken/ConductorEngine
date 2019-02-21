#include <mesh/SkeletonMatrixCollectionSystem.h>

#include <ecs/EntityManager.h>
#include <scene/SceneTransformComponent.h>

namespace Mesh
{
void SkeletonMatrixCollectionSystem::Update(
	const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	deferredFunctions.Add(
		[ecsGroups](ECS::EntityManager& entityManager)
		{
			for (const auto& group : ecsGroups)
			{
				const auto& skeletonRootComponent = group.Get<const SkeletonRootComponent>();
				auto& meshComponent = group.Get<MeshComponent>();

				const uint32_t numBones = skeletonRootComponent.m_boneTransformComponentIDs.Size();
				meshComponent.m_boneToWorldMatrices.Resize(numBones);

				for (size_t i = 0; i < numBones; ++i)
				{
					const ECS::ComponentID& boneTransformComponentID =
						skeletonRootComponent.m_boneTransformComponentIDs[i];
					AMP_FATAL_ASSERT(boneTransformComponentID.GetType() == Scene::SceneTransformComponent::k_type,
						"Encountered a bone transform component ID which wasn't for a SceneTransformComponent.");

					const auto* const boneTransformComponent = static_cast<const Scene::SceneTransformComponent*>(
						entityManager.FindComponent(boneTransformComponentID));
					if (boneTransformComponent != nullptr)
					{
						meshComponent.m_boneToWorldMatrices[i] = boneTransformComponent->m_modelToWorldMatrix;
					}
					else
					{
						meshComponent.m_boneToWorldMatrices[i] = Math::Matrix4x4();
					}
				}
			}
		});
}
}
