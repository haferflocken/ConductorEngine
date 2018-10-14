#include <behave/nodes/SelectorNode.h>

#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <behave/parse/BehaveParsedTree.h>

namespace Internal_SelectorNode
{
using namespace Behave;
using namespace Behave::Nodes;

class SelectorBehaviourState final : public BehaviourNodeState
{
public:
	explicit SelectorBehaviourState(const SelectorNode& node)
		: m_node(&node)
		, m_activeChildIndex(0)
	{}

	virtual const BehaviourNode* GetNode() const override { return m_node; }

	virtual EvaluateResult Evaluate(ECS::Entity& entity, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions,
		const BehaveContext& context) override
	{
		// If the active child is the max value of size_t, return Success.
		if (m_activeChildIndex == std::numeric_limits<size_t>::max())
		{
			return EvaluateResult::Success;
		}
		// If this node runs out of children to evaluate, return Failure.
		if (m_activeChildIndex >= m_node->GetChildCount())
		{
			return EvaluateResult::Failure;
		}

		// Evaluate the active child.
		const BehaviourNode* const activeChild = m_node->GetChildAt(m_activeChildIndex);
		activeChild->PushState(treeEvaluator);
		return EvaluateResult::PushedNode;
	}

	virtual void NotifyChildFinished(const BehaviourNode* child, const EvaluateResult result) override
	{
		Dev::FatalAssert(child == m_node->GetChildAt(m_activeChildIndex),
			"A selector should never receive a result from any node other than its active child.");

		switch (result)
		{
		case EvaluateResult::Success:
		{
			// Make this node return Success at the next call to Evaluate().
			m_activeChildIndex = std::numeric_limits<size_t>::max();
			break;
		}
		case EvaluateResult::Failure:
		{
			// Make this node evaluate the next child at the next call to Evaluate().
			++m_activeChildIndex;
			break;
		}
		case EvaluateResult::Running:
		case EvaluateResult::PushedNode:
		case EvaluateResult::Return:
		{
			Dev::FatalError("Invalid result type [%d] for selector.", static_cast<int32_t>(result));
			break;
		}
		default:
		{
			Dev::FatalError("Unknown result type [%d].", static_cast<int32_t>(result));
			break;
		}
		}
	}

private:
	const SelectorNode* m_node;
	size_t m_activeChildIndex;
};
}

Mem::UniquePtr<Behave::BehaviourNode> Behave::Nodes::SelectorNode::CreateFromNodeExpression(
	const BehaviourNodeFactory& nodeFactory,
	const AST::Interpreter& interpreter,
	Parse::NodeExpression& nodeExpression,
	const BehaviourTree& tree)
{
	auto node = Mem::MakeUnique<SelectorNode>(tree);

	if (!nodeFactory.TryMakeNodesFrom(nodeExpression.m_arguments, tree, node->m_children))
	{
		return nullptr;
	}

	return node;
}

void Behave::Nodes::SelectorNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<Internal_SelectorNode::SelectorBehaviourState>(*this);
}
