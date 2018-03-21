#pragma once

#include <behave/BehaviourSystem.h>

#include <functional>

namespace Collection
{
template <typename T> class ArrayView;
template <typename T> class Vector;
}

namespace Behave
{
class Actor;
class ActorManager;
class BehaveContext;
namespace Components { class BehaviourTreeComponent; }

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
	void Update(ActorManager& actorManager, const BehaveContext& context,
		const Collection::ArrayView<ActorComponentGroupType>& actorComponentGroups,
		Collection::Vector<std::function<void()>>& deferredFunctions) const;
};
}
}
