#pragma once

#include <ecs/Entity.h>
#include <ecs/System.h>

#include <condui/UITransformComponent.h>

namespace Condui
{
/**
 * The RelativeUITransformSystem updates the UI transforms of entities in parent/child relationships.
 * An entity's transform is relative to their parent's transform.
 */
class RelativeUITransformSystem final : public ECS::SystemTempl<
	Util::TypeList<ECS::Entity>,
	Util::TypeList<UITransformComponent>>
{
public:
	RelativeUITransformSystem() = default;
	virtual ~RelativeUITransformSystem() {}

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

private:
	void ProcessUpdateBuffer(ECS::EntityManager& entityManager);

	struct UpdateEntry
	{
		const ECS::Entity* m_entity;
		UITransformComponent* m_component;
	};
	Collection::Vector<UpdateEntry> m_updateBuffer;
};
}
