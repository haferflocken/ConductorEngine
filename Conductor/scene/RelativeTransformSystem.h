#pragma once

#include <ecs/System.h>

#include <scene/SceneTransformComponent.h>
#include <scene/SceneTransformComponentInfo.h>

namespace Scene
{
/**
 * The RelativeTransformSystem updates the scene transforms of entities in parent/child relationships.
 * An entity's transform is relative to their parent's transform.
 */
class RelativeTransformSystem final : public ECS::SystemTempl<
	Util::TypeList<ECS::Entity>,
	Util::TypeList<SceneTransformComponent>>
{
public:
	RelativeTransformSystem() = default;
	virtual ~RelativeTransformSystem() {}

	void Update(const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

private:
	void ProcessUpdateBuffer(ECS::EntityManager& entityManager);

	struct UpdateEntry
	{
		const ECS::Entity* m_entity;
		SceneTransformComponent* m_component;
	};
	Collection::Vector<UpdateEntry> m_updateBuffer;
};
}
