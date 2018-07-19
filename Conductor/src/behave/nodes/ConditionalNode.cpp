#include <behave/nodes/ConditionalNode.h>

#include <behave/BehaviourCondition.h>
#include <behave/BehaviourNodeFactory.h>
#include <behave/BehaviourNodeState.h>
#include <behave/BehaviourTreeEvaluator.h>

#include <json/JSONTypes.h>

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
				if (condition->Check(entity))
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

	virtual void NotifyChildFinished(const BehaviourNode* child, const EvaluateResult result)
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

Mem::UniquePtr<Behave::BehaviourNode> Behave::Nodes::ConditionalNode::LoadFromJSON(
	const Behave::BehaviourNodeFactory& nodeFactory,
	const JSON::JSONObject& jsonObject,
	const BehaviourTree& tree)
{
	using namespace Internal_ConditionalNode;

	const JSON::JSONArray* const children = jsonObject.FindArray(k_childrenHash);
	if (children == nullptr)
	{
		Dev::LogWarning("Failed to find an array at \"children\".");
		return nullptr;
	}

	auto node = Mem::MakeUnique<ConditionalNode>(tree);
	for (const auto& entry : *children)
	{
		if (entry->GetType() != JSON::ValueType::Object)
		{
			Dev::LogWarning("Encountered a non-object value within \"children\".");
			return nullptr;
		}

		const JSON::JSONObject* const pair = static_cast<const JSON::JSONObject*>(entry.Get());
		const JSON::JSONObject* const condition = pair->FindObject(k_conditionHash);
		if (condition == nullptr)
		{
			Dev::LogWarning("Failed to find a condition within a conditional node's child.");
			return nullptr;
		}
		
		const JSON::JSONObject* const childNode = pair->FindObject(k_nodeHash);
		if (childNode == nullptr)
		{
			Dev::LogWarning("Failed to find a node within a conditional node's child.");
			return nullptr;
		}

		node->m_conditions.Add(nodeFactory.MakeCondition(*condition));
		node->m_children.Add(nodeFactory.MakeNode(*childNode, tree));
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
