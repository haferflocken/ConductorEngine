#include <behave/nodes/RepeatNode.h>

#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <behave/parse/BehaveParsedTree.h>

namespace Behave
{
namespace Internal_RepeatNode
{
class RepeatBehaviourState final : public BehaviourNodeState
{
public:
	RepeatBehaviourState(const Nodes::RepeatNode& node)
		: m_node(&node)
		, m_childResult(EvaluateResult::Running)
	{}

	virtual const BehaviourNode* GetNode() const override { return m_node; }

	virtual EvaluateResult Evaluate(ECS::Entity& entity, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions,
		const BehaveContext& context) override
	{
		switch (m_childResult)
		{
		case EvaluateResult::Running:
		{
			m_node->GetChild()->PushState(treeEvaluator);
			return EvaluateResult::PushedNode;
		}
		case EvaluateResult::Success:
		{
			m_childResult = EvaluateResult::Running;
			return EvaluateResult::Running;
		}
		case EvaluateResult::Failure:
		{
			return EvaluateResult::Failure;
		}
		case EvaluateResult::PushedNode:
		case EvaluateResult::Return:
		{
			Dev::FatalError("Invalid code path encountered: child result should never be PushedNode or Return.");
			return EvaluateResult::Failure;
		}
		default:
		{
			Dev::FatalError("Unknown result type [%d].", static_cast<int32_t>(m_childResult));
			return EvaluateResult::Failure;
		}
		}
	}

	virtual void NotifyChildFinished(const BehaviourNode* child, const EvaluateResult result) override
	{
		m_childResult = result;
	}

private:
	const Nodes::RepeatNode* m_node;
	EvaluateResult m_childResult;
};
}

Mem::UniquePtr<BehaviourNode> Nodes::RepeatNode::CreateFromNodeExpression(const BehaviourNodeFactory& nodeFactory,
	const AST::Interpreter& interpreter, Parse::NodeExpression& nodeExpression, const BehaviourTree& tree)
{
	if (nodeExpression.m_arguments.Size() != 1 || !nodeExpression.m_arguments.Front().Is<Parse::NodeExpression>())
	{
		Dev::LogWarning("Repeat nodes take only one argument: a node expression.");
		return nullptr;
	}

	Mem::UniquePtr<BehaviourNode> child = nodeFactory.MakeNode(
		nodeExpression.m_arguments.Front().Get<Parse::NodeExpression>(), tree);
	if (child == nullptr)
	{
		return nullptr;
	}

	return Mem::MakeUnique<RepeatNode>(tree, std::move(child));
}

Nodes::RepeatNode::RepeatNode(const BehaviourTree& tree, Mem::UniquePtr<BehaviourNode>&& child)
	: BehaviourNode(tree)
	, m_child(std::move(child))
{}

Nodes::RepeatNode::~RepeatNode()
{}

void Nodes::RepeatNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<Internal_RepeatNode::RepeatBehaviourState>(*this);
}

const BehaviourNode* Nodes::RepeatNode::GetChild() const
{
	return m_child.Get();
}
}
