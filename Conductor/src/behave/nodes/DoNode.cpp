#include <behave/nodes/DoNode.h>

#include <behave/ast/ASTTypes.h>
#include <behave/ast/Interpreter.h>
#include <behave/BehaveContext.h>
#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <behave/parse/BehaveParsedTree.h>

namespace Behave
{
namespace Internal_DoNode
{
class DoNodeBehaviourState final : public BehaviourNodeState
{
public:
	explicit DoNodeBehaviourState(const Nodes::DoNode& node)
		: m_node(&node)
	{}

	virtual const BehaviourNode* GetNode() const override { return m_node; }

	virtual EvaluateResult Evaluate(ECS::Entity& entity, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void()>>& deferredFunctions,
		const BehaveContext& context) override
	{
		for (const auto& expression : m_node->GetExpressions())
		{
			context.m_interpreter.EvaluateExpression(expression, context.m_entityManager, entity);
		}
		return EvaluateResult::Success;
	}

private:
	const Nodes::DoNode* m_node;
};
}

Mem::UniquePtr<BehaviourNode> Nodes::DoNode::CreateFromNodeExpression(
	const BehaviourNodeFactory& nodeFactory,
	const AST::Interpreter& interpreter,
	const Parse::NodeExpression& nodeExpression,
	const BehaviourTree& tree)
{
	Collection::Vector<AST::Expression> expressions;

	for (const auto& expression : nodeExpression.m_arguments)
	{
		AST::ExpressionCompileResult compileResult = interpreter.Compile(expression);
		if (!compileResult.Is<AST::Expression>())
		{
			const AST::TypeCheckFailure& typeCheckFailure = compileResult.Get<AST::TypeCheckFailure>();
			Dev::LogWarning("Type Checking Failure: %s", typeCheckFailure.m_message.c_str());
			return nullptr;
		}
		expressions.Add(std::move(compileResult.Get<AST::Expression>()));
	}

	return Mem::MakeUnique<DoNode>(tree, std::move(expressions));
}

Nodes::DoNode::DoNode(const BehaviourTree& tree, Collection::Vector<AST::Expression>&& expressions)
	: BehaviourNode(tree)
	, m_expressions(std::move(expressions))
{}

Nodes::DoNode::~DoNode()
{}

void Nodes::DoNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<Internal_DoNode::DoNodeBehaviourState>(*this);
}
}
