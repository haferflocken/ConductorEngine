#pragma once

#include <ecs/BehaviourSystem.h>
#include <ecs/components/BehaviourTreeComponent.h>
#include <ecs/components/BehaviourTreeComponentInfo.h>

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
class Actor;
class ActorManager;

namespace Systems
{
/**
 * The behaviour tree evaluation system evaluates the behaviour trees of actors.
 */
class BehaviourTreeEvaluationSystem : public BehaviourSystemTempl<
	Util::TypeList<>,
	Util::TypeList<Actor, Components::BehaviourTreeComponent>>
{
public:
	void Update(ActorManager& actorManager, const Behave::BehaveContext& context,
		const Collection::ArrayView<ActorComponentGroupType>& actorComponentGroups,
		Collection::Vector<std::function<void()>>& deferredFunctions) const;
};
}
}
