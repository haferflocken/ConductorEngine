#pragma once

#include <ecs/components/BehaviourTreeComponent.h>
#include <ecs/components/BehaviourTreeComponentInfo.h>
#include <ecs/Entity.h>
#include <ecs/System.h>

#include <functional>

namespace Behave
{
class BehaveContext;
}

namespace Collection
{
template <typename T> class ArrayView;
template <typename T> class Vector;
}

namespace ECS
{
class Entity;
class EntityManager;

namespace Systems
{
/**
 * The behaviour tree evaluation system evaluates the behaviour trees of entities.
 */
class BehaviourTreeEvaluationSystem : public SystemTempl<
	Util::TypeList<>,
	Util::TypeList<Entity, Components::BehaviourTreeComponent>>
{
public:
	void Update(EntityManager& actorManager, const Behave::BehaveContext& context,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void()>>& deferredFunctions) const;
};
}
}
