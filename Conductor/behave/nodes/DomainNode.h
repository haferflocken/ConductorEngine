#pragma once

#include <behave/BehaviourNode.h>
#include <mem/UniquePtr.h>

namespace Behave
{
class BehaviourCondition;

namespace Nodes
{
class DomainNode final : public BehaviourNode
{
public:
	static constexpr const char* k_dslName = "domain";

	static Mem::UniquePtr<BehaviourNode> CreateFromNodeExpression(const BehaviourNodeFactory& nodeFactory,
		const Parse::NodeExpression& nodeExpression, const BehaviourTree& tree);

	explicit DomainNode(const BehaviourTree& tree);
	virtual ~DomainNode();

	virtual void PushState(BehaviourTreeEvaluator& treeEvaluator) const override;

	const BehaviourCondition* GetCondition() const { return m_condition.Get(); }
	const BehaviourNode* GetChild() const { return m_child.Get(); }

private:
	Mem::UniquePtr<BehaviourCondition> m_condition;
	Mem::UniquePtr<BehaviourNode> m_child;
};
}
}
