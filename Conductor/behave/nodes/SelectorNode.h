#pragma once

#include <behave/BehaviourNode.h>
#include <collection/Vector.h>
#include <mem/UniquePtr.h>

namespace Behave
{
namespace Nodes
{
class SelectorNode final : public BehaviourNode
{
public:
	static Mem::UniquePtr<BehaviourNode> LoadFromJSON(const BehaviourNodeFactory& nodeFactory,
		const JSON::JSONObject& jsonObject, const BehaviourTree& tree);

	explicit SelectorNode(const BehaviourTree& tree)
		: BehaviourNode(tree)
		, m_children()
	{}

	virtual ~SelectorNode() {}

	virtual void PushState(BehaviourTreeEvaluator& treeEvaluator) const override;

	const BehaviourNode* GetChildAt(const size_t i) const { return m_children[i].Get(); }
	size_t GetChildCount() const { return m_children.Size(); }

private:
	Collection::Vector<Mem::UniquePtr<BehaviourNode>> m_children;
};
}
}
