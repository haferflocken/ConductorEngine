#pragma once

namespace JSON
{
class JSONObject;
}

namespace Behave
{
class BehaviourNodeFactory;
class BehaviourTree;
class BehaviourTreeEvaluator;

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
