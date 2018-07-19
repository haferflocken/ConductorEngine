#include <behave/nodes/ReturnNode.h>

#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTreeEvaluator.h>

#include <json/JSONTypes.h>

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

	virtual EvaluateResult Evaluate(ECS::Actor& actor, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void()>>& deferredFunctions,
		const BehaveContext& context) override
	{
		return EvaluateResult::Return;
	}

private:
	const ReturnNode* m_node;
};

const Util::StringHash k_returnsSuccessHash = Util::CalcHash("returns_success");
}

Mem::UniquePtr<Behave::BehaviourNode> Behave::Nodes::ReturnNode::LoadFromJSON(const BehaviourNodeFactory& nodeFactory,
	const JSON::JSONObject& jsonObject, const BehaviourTree& tree)
{
	const JSON::JSONBoolean* const returnsSuccess = jsonObject.FindBoolean(Internal_ReturnNode::k_returnsSuccessHash);
	if (returnsSuccess != nullptr)
	{
		auto node = Mem::MakeUnique<ReturnNode>(tree);
		node->m_returnsSuccess = returnsSuccess->m_boolean;
		return node;
	}
	return nullptr;
}

void Behave::Nodes::ReturnNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<Internal_ReturnNode::ReturnBehaviourState>(*this);
}
