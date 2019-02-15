#include <condui/StackingPanelSystem.h>

#include <ecs/EntityManager.h>

namespace Condui
{
void StackingPanelSystem::Update(
	const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	// Defer the update to get access to the entity manager.
	deferredFunctions.Add([this, ecsGroups](ECS::EntityManager& entityManager)
		{
			DeferredUpdate(entityManager, ecsGroups);
		});
}

void StackingPanelSystem::DeferredUpdate(
	ECS::EntityManager& entityManager,
	const Collection::ArrayView<ECSGroupType>& ecsGroups) const
{
	for (const auto& ecsGroup : ecsGroups)
	{
		const auto& entity = ecsGroup.Get<const ECS::Entity>();
		auto& sceneTransformComponent = ecsGroup.Get<Scene::SceneTransformComponent>();

		const ECS::Entity* const parentEntity = entity.GetParent();
		if (parentEntity == nullptr)
		{
			continue;
		}

		const Scene::SceneTransformComponent* const parentTransformComponent =
			entityManager.FindComponent<Scene::SceneTransformComponent>(*parentEntity);
		if (parentTransformComponent == nullptr)
		{
			continue;
		}

		const Math::Matrix4x4& parentTransform = parentTransformComponent->m_modelToWorldMatrix;
		sceneTransformComponent.m_modelToWorldMatrix = parentTransform * sceneTransformComponent.m_childToParentMatrix;
	}
}
}
