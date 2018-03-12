#pragma once

#include <behave/BehaviourNode.h>
#include <collection/Vector.h>
#include <mem/UniquePtr.h>

namespace Behave
{
class BehaviourCondition;

namespace Nodes
{
class ConditionalNode final : public BehaviourNode
{
public:
	static Mem::UniquePtr<BehaviourNode> LoadFromJSON(const BehaviourNodeFactory& nodeFactory,
		const JSON::JSONObject& jsonObject, const BehaviourTree& tree);

	explicit ConditionalNode(const BehaviourTree& tree);
	virtual ~ConditionalNode();

	virtual void PushState(BehaviourTreeEvaluator& treeEvaluator) const override;

	const BehaviourCondition* GetCondition(const size_t i) const { return m_conditions[i].Get(); }
	const BehaviourNode* GetChild(const size_t i) const { return m_children[i].Get(); }

	size_t GetChildCount() const { return m_children.Size(); }

private:
	Collection::Vector<Mem::UniquePtr<BehaviourCondition>> m_conditions;
	Collection::Vector<Mem::UniquePtr<BehaviourNode>> m_children;
};
}
}
