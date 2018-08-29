#pragma once

namespace Behave
{
namespace AST { class Interpreter; }
class BehaviourNodeFactory;
class BehaviourTree;
class BehaviourTreeEvaluator;

namespace Parse { struct NodeExpression; }

class BehaviourNode
{
public:
	explicit BehaviourNode(const BehaviourTree& tree)
		: m_tree(tree)
	{}

	virtual ~BehaviourNode() {}

	const BehaviourTree& GetTree() const { return m_tree; }

	// Instantiates a BehaviourNodeState for this node on the evaluator's call stack.
	virtual void PushState(BehaviourTreeEvaluator& treeEvaluator) const = 0;

private:
	const BehaviourTree& m_tree;
};
}
