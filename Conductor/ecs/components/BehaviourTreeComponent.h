#pragma once

#include <behave/BehaviourTreeEvaluator.h>
#include <ecs/ActorComponent.h>

namespace ECS
{
class ActorComponentVector;

namespace Components
{
class BehaviourTreeComponentInfo;

/**
 * A BehaviourTreeComponent allows an Actor to run behaviour trees.
 */
class BehaviourTreeComponent final : public ActorComponent
{
public:
	using Info = BehaviourTreeComponentInfo;

	static bool TryCreateFromInfo(const BehaviourTreeComponentInfo& componentInfo, const ActorComponentID reservedID,
		ActorComponentVector& destination);

	explicit BehaviourTreeComponent(const ActorComponentID id)
		: ActorComponent(id)
		, m_treeEvaluators()
	{}

	BehaviourTreeComponent(const BehaviourTreeComponent&) = delete;
	BehaviourTreeComponent& operator=(const BehaviourTreeComponent&) = delete;

	BehaviourTreeComponent(BehaviourTreeComponent&&) = default;
	BehaviourTreeComponent& operator=(BehaviourTreeComponent&&) = default;

	virtual ~BehaviourTreeComponent() {}

	Collection::Vector<Behave::BehaviourTreeEvaluator> m_treeEvaluators;
};
}
}
