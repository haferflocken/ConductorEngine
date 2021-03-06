#include <behave/nodes/LogNode.h>

#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTree.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <behave/parse/BehaveParsedTree.h>

#include <dev/Dev.h>

#include <mem/UniquePtr.h>

namespace Internal_LogNode
{
using namespace Behave;
using namespace Behave::Nodes;

class LogBehaviourState final : public BehaviourNodeState
{
public:
	explicit LogBehaviourState(const LogNode& node)
		: m_node(&node)
	{}

	virtual const BehaviourNode* GetNode() const override { return m_node; }

	virtual EvaluateResult Evaluate(const BehaveContext& context,
		const Collection::Vector<Asset::AssetHandle<BehaviourForest>>& forests,
		ECS::Entity& entity,
		BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) override
	{
		AMP_LOG("%s", m_node->GetMessage());
		return EvaluateResult::Success;
	}

private:
	const LogNode* m_node;
};
}

Mem::UniquePtr<Behave::BehaviourNode> Behave::Nodes::LogNode::CreateFromNodeExpression(
	const BehaviourNodeFactory& nodeFactory,
	const AST::Interpreter& interpreter,
	Parse::NodeExpression& nodeExpression,
	const BehaviourTree& tree)
{
	if (nodeExpression.m_arguments.Size() != 1
		|| !nodeExpression.m_arguments.Front().Is<Parse::LiteralExpression>())
	{
		AMP_LOG_WARNING("Log nodes take only one argument: a string literal.");
		return nullptr;
	}

	const auto& literalExpression = nodeExpression.m_arguments.Front().Get<Parse::LiteralExpression>();
	if (!literalExpression.Is<Parse::StringLiteral>())
	{
		AMP_LOG_WARNING("Log nodes take only one argument: a string literal.");
		return nullptr;
	}

	auto node = Mem::MakeUnique<LogNode>(tree);
	node->m_message = literalExpression.Get<Parse::StringLiteral>().m_value;
	return node;
}

void Behave::Nodes::LogNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<Internal_LogNode::LogBehaviourState>(*this);
}
