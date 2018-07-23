// GENERATED CODE
#pragma once

#include <ecs/Component.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <collection/Vector.h>
#include <cstdint>
#include <string>

namespace ECS { class ComponentVector; }

namespace Behave::Components
{
class BehaviourTreeComponentInfo;

/**
 * A BehaviourTreeComponent allows an Entity to run behaviour trees.
 */
class BehaviourTreeComponent final : public ECS::Component
{
public:
	using Info = BehaviourTreeComponentInfo;
	static bool TryCreateFromInfo(const Info& componentInfo, 
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);
	
	explicit BehaviourTreeComponent(const ECS::ComponentID id)
		: ECS::Component(id)
	{}
	
	BehaviourTreeComponent(const BehaviourTreeComponent&) = delete;
	BehaviourTreeComponent& operator=(const BehaviourTreeComponent&) = delete;
	
	BehaviourTreeComponent(BehaviourTreeComponent&&) = default;
	BehaviourTreeComponent& operator=(BehaviourTreeComponent&&) = default;
	
	virtual ~BehaviourTreeComponent() {}
	
	Collection::Vector<BehaviourTreeEvaluator> m_treeEvaluators;
};
}
