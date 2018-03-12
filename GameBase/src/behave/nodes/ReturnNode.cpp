#include <behave/nodes/ReturnNode.h>

#include <behave/Actor.h>
#include <behave/BehaviourNodeState.h>

#include <json/JSONTypes.h>

#include <mem/UniquePtr.h>

using namespace Behave;
using namespace Behave::Nodes;

namespace
{
class ReturnBehaviourState final : public BehaviourNodeState
{
public:
	explicit ReturnBehaviourState(const ReturnNode& node)
		: m_node(&node)
	{}

	virtual const BehaviourNode* GetNode() const override { return m_node; }

	virtual EvaluateResult Evaluate(Actor& actor, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void()>>& deferredFunctions,
		const BehaviourTreeContext& context) override
	{
		return EvaluateResult::Return;
	}

private:
	const ReturnNode* m_node;
};

const Util::StringHash k_returnsSuccessHash = Util::CalcHash("returns_success");
}

Mem::UniquePtr<BehaviourNode> ReturnNode::LoadFromJSON(const BehaviourNodeFactory& nodeFactory,
	const JSON::JSONObject& jsonObject, const BehaviourTree& tree)
{
	const JSON::JSONBoolean* const returnsSuccess = jsonObject.FindBoolean(k_returnsSuccessHash);
	if (returnsSuccess != nullptr)
	{
		auto node = Mem::MakeUnique<ReturnNode>(tree);
		node->m_returnsSuccess = returnsSuccess->m_boolean;
		return node;
	}
	return nullptr;
}

void ReturnNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<ReturnBehaviourState>(*this);
}
