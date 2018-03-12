#include <behave/nodes/SequenceNode.h>

#include <behave/Actor.h>
#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourNodeState.h>

using namespace Behave;
using namespace Behave::Nodes;

namespace
{
class SequenceBehaviourState final : public BehaviourNodeState
{
public:
	explicit SequenceBehaviourState(const SequenceNode& node)
		: m_node(&node)
		, m_activeChildIndex(0)
	{}

	virtual const BehaviourNode* GetNode() const override { return m_node; }

	virtual EvaluateResult Evaluate(Actor& actor, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void()>>& deferredFunctions,
		const BehaviourTreeContext& context) override
	{
		// If the active child is the max value of size_t, return Failure.
		if (m_activeChildIndex == std::numeric_limits<size_t>::max())
		{
			return EvaluateResult::Failure;
		}
		// If this node runs out of children to evaluate, return Success.
		if (m_activeChildIndex >= m_node->GetChildCount())
		{
			return EvaluateResult::Success;
		}

		// Evaluate the active child.
		const BehaviourNode* const activeChild = m_node->GetChildAt(m_activeChildIndex);
		activeChild->PushState(treeEvaluator);
		return EvaluateResult::PushedNode;
	}

	virtual void NotifyChildFinished(const BehaviourNode* child, const EvaluateResult result) override
	{
		Dev::FatalAssert(child == m_node->GetChildAt(m_activeChildIndex),
			"A sequence should never receive a result from any node other than its active child.");

		switch (result)
		{
		case EvaluateResult::Success:
		{
			// Make this node evaluate the next child at the next call to Evaluate().
			++m_activeChildIndex;
			break;
		}
		case EvaluateResult::Failure:
		{
			// Make this node return Failure at the next call to Evaluate().
			m_activeChildIndex = std::numeric_limits<size_t>::max();
			break;
		}
		case EvaluateResult::Running:
		case EvaluateResult::PushedNode:
		case EvaluateResult::Return:
		{
			Dev::FatalError("Invalid result type [%d] for sequence.", static_cast<int32_t>(result));
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
	const SequenceNode* m_node;
	size_t m_activeChildIndex;
};

const Util::StringHash k_childrenHash = Util::CalcHash("children");
}

Mem::UniquePtr<BehaviourNode> SequenceNode::LoadFromJSON(const BehaviourNodeFactory& nodeFactory,
	const JSON::JSONObject& jsonObject, const BehaviourTree& tree)
{
	auto node = Mem::MakeUnique<SequenceNode>(tree);
	
	if (!nodeFactory.TryMakeNodesFrom(jsonObject, tree, k_childrenHash, node->m_children))
	{
		return nullptr;
	}

	return node;
}

void SequenceNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<SequenceBehaviourState>(*this);
}
