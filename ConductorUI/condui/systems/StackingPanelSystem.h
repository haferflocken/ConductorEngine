#pragma once

#include <condui/components/StackingPanelComponent.h>
#include <ecs/Entity.h>
#include <ecs/System.h>
#include <scene/SceneTransformComponent.h>

namespace Condui
{
/**
 * The StackingPanelSystem vertically stacks the children of entities with StackingPanelComponents.
 */
class StackingPanelSystem final : public ECS::SystemTempl<
	Util::TypeList<ECS::Entity, Condui::StackingPanelComponent>,
	Util::TypeList<Scene::SceneTransformComponent>>
{
public:
	StackingPanelSystem() = default;
	virtual ~StackingPanelSystem() {}

	void Update(
		const Unit::Time::Millisecond delta,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const;

private:
	void DeferredUpdate(ECS::EntityManager& entityManager, const Collection::ArrayView<ECSGroupType>& ecsGroups) const;
};
}
