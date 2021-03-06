#include <behave/nodes/DomainNode.h>

#include <behave/BehaviourCondition.h>
#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <behave/parse/BehaveParsedTree.h>

namespace Internal_DomainNode
{
using namespace Behave;
using namespace Behave::Nodes;

class DomainBehaviourState final : public BehaviourNodeState
{
public:
	DomainBehaviourState(const DomainNode& node)
		: m_node(&node)
		, m_childResult(EvaluateResult::Running)
	{}

	virtual const BehaviourNode* GetNode() const override { return m_node; }

	virtual EvaluateResult Evaluate(const BehaveContext& context,
		const Collection::Vector<Asset::AssetHandle<BehaviourForest>>& forests,
		ECS::Entity& entity,
		BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) override
	{
		if (m_childResult != EvaluateResult::Running)
		{
			treeEvaluator.GetDomainStack().RemoveLast();
			return m_childResult;
		}

		treeEvaluator.GetDomainStack().Emplace(this, m_node->GetCondition());

		m_node->GetChild()->PushState(treeEvaluator);
		return EvaluateResult::PushedNode;
	}

	virtual void NotifyChildFinished(const BehaviourNode* child, const EvaluateResult result) override
	{
		if (child == nullptr)
		{
			// A null child signals that the domain condition failed and the entity terminated this node's subtree.
			// Set m_childResult to failure so that this node returns Failure on the next call to Evaluate().
			m_childResult = EvaluateResult::Failure;
		}
		else
		{
			// If the child is not null, capture its result so that this node returns it on the next call to Evaluate().
			AMP_FATAL_ASSERT(child == m_node, "A domain should never receive a result from any node except its child.");
			m_childResult = result;
		}
	}

	virtual void NotifyEarlyTermination(BehaviourTreeEvaluator& treeEvaluator) override
	{
		// Early terminating a domain behaviour requires the domain behaviour to pop itself from the domain stack.
		// This will pop domains from the domain stack in the correct order because the early terminations happen as
		// nodes are popped from the call stack.
		treeEvaluator.GetDomainStack().RemoveLast();
	}

private:
	const DomainNode* m_node;
	EvaluateResult m_childResult;
};
}

Mem::UniquePtr<Behave::BehaviourNode> Behave::Nodes::DomainNode::CreateFromNodeExpression(
	const BehaviourNodeFactory& nodeFactory,
	const AST::Interpreter& interpreter,
	Parse::NodeExpression& nodeExpression,
	const BehaviourTree& tree)
{
	if (nodeExpression.m_arguments.Size() != 2)
	{
		AMP_LOG_WARNING("Domain nodes take exactly two arguments: a condition and node expression.");
		return nullptr;
	}

	Mem::UniquePtr<BehaviourCondition> condition = nodeFactory.MakeCondition(nodeExpression.m_arguments.Front());
	if (condition == nullptr)
	{
		return nullptr;
	}

	if (!nodeExpression.m_arguments.Back().Is<Parse::NodeExpression>())
	{
		AMP_LOG_WARNING("Domain nodes require a node expression as their second argument.");
		return nullptr;
	}

	Mem::UniquePtr<BehaviourNode> child = nodeFactory.MakeNode(
		nodeExpression.m_arguments.Back().Get<Parse::NodeExpression>(), tree);
	if (child == nullptr)
	{
		return nullptr;
	}

	auto node = Mem::MakeUnique<DomainNode>(tree);
	node->m_condition = std::move(condition);
	node->m_child = std::move(child);

	return node;
}

Behave::Nodes::DomainNode::DomainNode(const BehaviourTree& tree)
	: BehaviourNode(tree)
	, m_condition()
	, m_child()
{}

Behave::Nodes::DomainNode::~DomainNode()
{}

void Behave::Nodes::DomainNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<Internal_DomainNode::DomainBehaviourState>(*this);
}
