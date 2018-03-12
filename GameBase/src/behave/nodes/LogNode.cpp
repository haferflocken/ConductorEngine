#include <behave/nodes/LogNode.h>

#include <behave/Actor.h>
#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTree.h>

#include <dev/Dev.h>

#include <json/JSONTypes.h>

#include <mem/UniquePtr.h>

using namespace Behave;
using namespace Behave::Nodes;

namespace
{
class LogBehaviourState final : public BehaviourNodeState
{
public:
	explicit LogBehaviourState(const LogNode& node)
		: m_node(&node)
	{}

	virtual const BehaviourNode* GetNode() const override { return m_node; }

	virtual EvaluateResult Evaluate(Actor& actor, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void()>>& deferredFunctions,
		const BehaviourTreeContext& context) override
	{
		Dev::Log("%s", m_node->GetMessage());
		return EvaluateResult::Success;
	}

private:
	const LogNode* m_node;
};

const Util::StringHash k_messageHash = Util::CalcHash("message");
}

Mem::UniquePtr<BehaviourNode> LogNode::LoadFromJSON(const BehaviourNodeFactory& nodeFactory,
	const JSON::JSONObject& jsonObject, const BehaviourTree& tree)
{
	const JSON::JSONString* const message = jsonObject.FindString(k_messageHash);
	if (message != nullptr)
	{
		auto node = Mem::MakeUnique<LogNode>(tree);
		node->m_message = message->m_string;
		return node;
	}
	return nullptr;
}

void LogNode::PushState(BehaviourTreeEvaluator& treeEvaluator) const
{
	treeEvaluator.GetCallStack().Emplace<LogBehaviourState>(*this);
}
