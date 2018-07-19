// GENERATED CODE
#pragma once

#include <ecs/ActorComponent.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <collection/Vector.h>
#include <cstdint>
#include <string>

namespace ECS { class ActorComponentVector; }

namespace Behave::Components
{
class BehaviourTreeComponentInfo;

/**
 * A BehaviourTreeComponent allows an Actor to run behaviour trees.
 */
class BehaviourTreeComponent final : public ECS::ActorComponent
{
public:
	using Info = BehaviourTreeComponentInfo;
	static bool TryCreateFromInfo(const Info& componentInfo, 
		const ActorComponentID reservedID, ActorComponentVector& destination);
	
	explicit BehaviourTreeComponent(const ECS::ActorComponentID id)
		: ECS::ActorComponent(id)
	{}
	
	BehaviourTreeComponent(const BehaviourTreeComponent&) = delete;
	BehaviourTreeComponent& operator=(const BehaviourTreeComponent&) = delete;
	
	BehaviourTreeComponent(BehaviourTreeComponent&&) = default;
	BehaviourTreeComponent& operator=(BehaviourTreeComponent&&) = default;
	
	virtual ~BehaviourTreeComponent() {}
	
	Collection::Vector<BehaviourTreeEvaluator> m_treeEvaluators;
};
}
