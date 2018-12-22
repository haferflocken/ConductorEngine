#include <condui/RelativeUITransformSystem.h>

#include <ecs/Entity.h>
#include <ecs/EntityManager.h>

#include <algorithm>

namespace Condui
{
void RelativeUITransformSystem::Update(const Unit::Time::Millisecond delta,
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
			UITransformComponent& component = ecsGroup.Get<UITransformComponent>();
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

void RelativeUITransformSystem::ProcessUpdateBuffer(ECS::EntityManager& entityManager)
{
	// Update the transform of each child to be relative to its parent's.
	for (const auto& updateEntry : m_updateBuffer)
	{
		UITransformComponent& component = *updateEntry.m_component;

		const UITransformComponent* const parentComponent =
			entityManager.FindComponent<UITransformComponent>(*updateEntry.m_entity->GetParent());
		if (parentComponent != nullptr)
		{
			const Math::Matrix4x4& parentTransform = parentComponent->m_uiTransform;
			component.m_uiTransform = parentTransform * component.m_transformFromParent;
		}
	}
}
}
