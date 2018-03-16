#include <behave/nodes/CallNode.h>

#include <behave/Actor.h>
#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTree.h>
#include <behave/BehaviourTreeContext.h>
#include <behave/BehaviourTreeManager.h>

#include <json/JSONTypes.h>

#include <mem/UniquePtr.h>

namespace Internal_CallNode
{
using namespace Behave;
using namespace Behave::Nodes;

class CallBehaviourState final : public BehaviourNodeState
{
public:
	explicit CallBehaviourState(const CallNode& node)
		: m_node(&node)
		, m_result(EvaluateResult::Running)
	{}

	virtual const BehaviourNode* GetNode() const override { return m_node; }

	virtual EvaluateResult Evaluate(Actor& actor, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void()>>& deferredFunctions,
		const BehaviourTreeContext& context) override
	{
		// Evaluate will be called twice during the lifetime of a CallBehaviourState.
		// First, Evaluate is called when the state is first encountered. In this case, m_result is Running,
		// and we push the called tree onto the stack. We return Running so that the called tree is not
		// evaluated until the next frame.
		// Second, Evaluate is called after the called tree returns. In this case, the state has already captured
		// the called tree's result in m_result, so we just returns that.
		if (m_result == EvaluateResult::Running)
		{
			const BehaviourTree* const treeToCall =
				context.m_behaviourTreeManager.FindTree(m_node->GetTreeToCall());
			if (treeToCall == nullptr)
			{
				Dev::LogWarning("Failed to resolve \"%s\" into a tree to call.",
					Util::ReverseHash(m_node->GetTreeToCall()));
				return EvaluateResult::Failure;
			}

			treeToCall->GetRoot()->PushState(treeEvaluator);
			return EvaluateResult::Running;
		}
		return m_result;
	}

	virtual void NotifyChildFinished(const BehaviourNode* child, const EvaluateResult result) override
	{
		Dev::FatalAssert(result != EvaluateResult::Running, "A finished child should never return 'running'.");
		m_result = result;
	}

private:
	const CallNode* m_node;
	EvaluateResult m_result;
};

const Util::StringHash k_treeToCallHash = Util::CalcHash("tree_to_call");
}

Mem::UniquePtr<Behave::BehaviourNode> Behave::Nodes::CallNode::LoadFromJSON(const BehaviourNodeFactory& nodeFactory,
	const JSON::JSONObject& jsonObject, const BehaviourTree& tree)
{
	const JSON::JSONString* const treeToCall = jsonObject.FindString(Internal_CallNode::k_treeToCallHash);
	if (treeToCall == nullptr)
	{
		Dev::LogWarning("CallBehaviour failed to find a tree to call.");
		return nullptr;
	}
	
	auto node = Mem::MakeUnique<CallNode>(tree);
	node->m_treeToCall = treeToCall->m_hash;
	return node;
}

void Behave::Nodes::CallNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<Internal_CallNode::CallBehaviourState>(*this);
}
