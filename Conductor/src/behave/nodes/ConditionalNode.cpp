#include <behave/nodes/ConditionalNode.h>

#include <behave/BehaveContext.h>
#include <behave/BehaviourCondition.h>
#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <behave/parse/BehaveParsedTree.h>

namespace Internal_ConditionalNode
{
using namespace Behave;
using namespace Behave::Nodes;

class ConditionalBehaviourState final : public BehaviourNodeState
{
public:
	explicit ConditionalBehaviourState(const ConditionalNode& node)
		: m_node(&node)
		, m_childResult(EvaluateResult::Running)
	{}

	virtual const BehaviourNode* GetNode() const override { return m_node; }

	virtual EvaluateResult Evaluate(ECS::Entity& entity, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void()>>& deferredFunctions,
		const BehaveContext& context) override
	{
		// If m_childResult is Running, it means that a child has not been evaluated yet.
		if (m_childResult == EvaluateResult::Running)
		{
			// Evaluate the first child that passes its condition.
			for (size_t i = 0, iEnd = m_node->GetChildCount(); i < iEnd; ++i)
			{
				const BehaviourCondition* const condition = m_node->GetCondition(i);
				if (condition->Check(context.m_interpreter, context.m_entityManager, entity))
				{
					const BehaviourNode* const childNode = m_node->GetChild(i);
					childNode->PushState(treeEvaluator);
					return EvaluateResult::PushedNode;
				}
			}
			// If all children fail their condition, return Failure.
			return EvaluateResult::Failure;
		}

		// If m_childResult is not Running, it means we have a child result. Return it.
		return m_childResult;
	}

	virtual void NotifyChildFinished(const BehaviourNode* child, const EvaluateResult result) override
	{
		// Capture the child's result so it can be returned at the next call to Evaluate().
		m_childResult = result;
	}

private:
	const ConditionalNode* m_node;
	EvaluateResult m_childResult;
};

const Util::StringHash k_childrenHash = Util::CalcHash("children");
const Util::StringHash k_conditionHash = Util::CalcHash("condition");
const Util::StringHash k_nodeHash = Util::CalcHash("node");
}

Mem::UniquePtr<Behave::BehaviourNode> Behave::Nodes::ConditionalNode::CreateFromNodeExpression(
	const Behave::BehaviourNodeFactory& nodeFactory,
	const AST::Interpreter& interpreter,
	const Parse::NodeExpression& nodeExpression,
	const BehaviourTree& tree)
{
	if (nodeExpression.m_arguments.IsEmpty() || (nodeExpression.m_arguments.Size() & 1) == 1)
	{
		Dev::LogWarning("Condition nodes require a positive, even number of arguments. "
			"Its arguments should be a alternating series of conditions and nodes.");
		return nullptr;
	}

	auto node = Mem::MakeUnique<ConditionalNode>(tree);

	for (size_t i = 0, iEnd = nodeExpression.m_arguments.Size(); i < iEnd; i += 2)
	{
		const auto& conditionExpression = nodeExpression.m_arguments[i];
		const auto& childExpression = nodeExpression.m_arguments[i + 1];

		if (!childExpression.Is<Parse::NodeExpression>())
		{
			Dev::LogWarning("Failed to create condition node: argument %zu was not a node expression.", i + 1);
			return nullptr;
		}

		const auto& childNodeExpression = childExpression.Get<Parse::NodeExpression>();

		Mem::UniquePtr<BehaviourCondition> condition = nodeFactory.MakeCondition(conditionExpression);
		if (condition == nullptr)
		{
			return nullptr;
		}

		Mem::UniquePtr<BehaviourNode> childNode = nodeFactory.MakeNode(childNodeExpression, tree);
		if (childNode == nullptr)
		{
			return nullptr;
		}

		node->m_conditions.Add(std::move(condition));
		node->m_children.Add(std::move(childNode));
	}

	return node;
}

Behave::Nodes::ConditionalNode::ConditionalNode(const BehaviourTree& tree)
	: BehaviourNode(tree)
	, m_conditions()
	, m_children()
{}

Behave::Nodes::ConditionalNode::~ConditionalNode()
{}

void Behave::Nodes::ConditionalNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<Internal_ConditionalNode::ConditionalBehaviourState>(*this);
}
