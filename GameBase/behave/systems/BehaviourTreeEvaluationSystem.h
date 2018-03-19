#pragma once

#include <behave/BehaviourSystem.h>

namespace Collection { template <typename T> class ArrayView; }

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
		const Collection::ArrayView<ActorComponentGroupType>& actorComponentGroups) const;
};
}
}
