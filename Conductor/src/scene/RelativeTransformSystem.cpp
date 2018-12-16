#include <scene/RelativeTransformSystem.h>

#include <ecs/Entity.h>
#include <ecs/EntityManager.h>

#include <algorithm>

namespace Scene
{
void RelativeTransformSystem::Update(const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions)
{
	// Sort the ECS groups by parent/child dependency and discard entities without parents.
	m_updateBuffer.Clear();
	for (const auto& ecsGroup : ecsGroups)
	{
		const ECS::Entity& entity = ecsGroup.Get<const ECS::Entity>();
		if (entity.GetParent() != nullptr)
		{
			SceneTransformComponent& component = ecsGroup.Get<SceneTransformComponent>();
			m_updateBuffer.Add({ &entity, &component });
		}
	}
	std::sort(m_updateBuffer.begin(), m_updateBuffer.end(), [](const UpdateEntry& lhs, const UpdateEntry& rhs)
	{
		return lhs.m_entity == rhs.m_entity->GetParent();
	});

	// Defer a function to process the update buffer so it has access to the entity manager.
	deferredFunctions.Add([this](ECS::EntityManager& entityManager)
	{
		ProcessUpdateBuffer(entityManager);
	});
}

void RelativeTransformSystem::ProcessUpdateBuffer(ECS::EntityManager& entityManager)
{
	// Update the transform of each child to be relative to its parent's.
	for (const auto& updateEntry : m_updateBuffer)
	{
		SceneTransformComponent& component = *updateEntry.m_component;

		const SceneTransformComponent* const parentComponent =
			entityManager.FindComponent<SceneTransformComponent>(*updateEntry.m_entity->GetParent());
		if (parentComponent != nullptr)
		{
			const Math::Matrix4x4& parentTransform = parentComponent->m_matrix;
			component.m_matrix = parentTransform * component.m_transformFromParentTransform;
		}
	}
}
}
