#pragma once

#include <behave/BehaviourTreeEvaluator.h>
#include <ecs/Component.h>

namespace ECS
{
class ComponentVector;

namespace Components
{
class BehaviourTreeComponentInfo;

/**
 * A BehaviourTreeComponent allows an Entity to run behaviour trees.
 */
class BehaviourTreeComponent final : public Component
{
public:
	using Info = BehaviourTreeComponentInfo;

	static bool TryCreateFromInfo(const BehaviourTreeComponentInfo& componentInfo, const ComponentID reservedID,
		ComponentVector& destination);

	explicit BehaviourTreeComponent(const ComponentID id)
		: Component(id)
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
