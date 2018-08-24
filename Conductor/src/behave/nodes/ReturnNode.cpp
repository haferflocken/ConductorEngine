#include <behave/nodes/ReturnNode.h>

#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <behave/parse/BehaveParsedTree.h>

#include <mem/UniquePtr.h>

namespace Internal_ReturnNode
{
using namespace Behave;
using namespace Behave::Nodes;

class ReturnBehaviourState final : public BehaviourNodeState
{
public:
	explicit ReturnBehaviourState(const ReturnNode& node)
		: m_node(&node)
	{}

	virtual const BehaviourNode* GetNode() const override { return m_node; }

	virtual EvaluateResult Evaluate(ECS::Entity& entity, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void()>>& deferredFunctions,
		const BehaveContext& context) override
	{
		return EvaluateResult::Return;
	}

private:
	const ReturnNode* m_node;
};
}

Mem::UniquePtr<Behave::BehaviourNode> Behave::Nodes::ReturnNode::CreateFromNodeExpression(
	const BehaviourNodeFactory& nodeFactory, const Parse::NodeExpression& nodeExpression, const BehaviourTree& tree)
{
	if (nodeExpression.m_arguments.Size() != 1
		|| !nodeExpression.m_arguments.Front().Is<Parse::LiteralExpression>())
	{
		Dev::LogWarning("Return nodes take only one argument: a result literal.");
		return nullptr;
	}

	const auto& literalExpression = nodeExpression.m_arguments.Front().Get<Parse::LiteralExpression>();
	if (!literalExpression.Is<Parse::ResultLiteral>())
	{
		Dev::LogWarning("Return nodes take only one argument: a result literal.");
		return nullptr;
	}

	auto node = Mem::MakeUnique<ReturnNode>(tree);
	node->m_returnsSuccess = literalExpression.Get<Parse::ResultLiteral>().m_isSuccess;
	return node;
}

void Behave::Nodes::ReturnNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<Internal_ReturnNode::ReturnBehaviourState>(*this);
}
