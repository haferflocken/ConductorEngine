#pragma once

#include <behave/BehaviourNode.h>
#include <mem/UniquePtr.h>

namespace Behave::Nodes
{
/**
 * RepeatNode repeatedly executes its child node as long as the child node results in Success.
 * If its child node fails, RepeatNode fails. This means that RepeatNode only terminates with Failure.
 */
class RepeatNode final : public BehaviourNode
{
public:
	static constexpr const char* k_dslName = "repeat";

	static Mem::UniquePtr<BehaviourNode> CreateFromNodeExpression(const BehaviourNodeFactory& nodeFactory,
		const AST::Interpreter& interpreter, const Parse::NodeExpression& nodeExpression, const BehaviourTree& tree);

	RepeatNode(const BehaviourTree& tree, Mem::UniquePtr<BehaviourNode>&& child);
	virtual ~RepeatNode();

	virtual void PushState(BehaviourTreeEvaluator& treeEvaluator) const override;

	const BehaviourNode* GetChild() const;

private:
	Mem::UniquePtr<BehaviourNode> m_child;
};
}
