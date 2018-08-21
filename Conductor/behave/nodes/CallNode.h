#pragma once

#include <behave/BehaviourNode.h>
#include <util/StringHash.h>

namespace Mem { template <typename T> class UniquePtr; }

namespace Behave::Nodes
{
class CallNode final : public BehaviourNode
{
public:
	static constexpr const char* k_dslName = "call";

	static Mem::UniquePtr<BehaviourNode> CreateFromNodeExpression(const BehaviourNodeFactory& nodeFactory,
		const Parse::NodeExpression& nodeExpression, const BehaviourTree& tree);

	explicit CallNode(const BehaviourTree& tree)
		: BehaviourNode(tree)
		, m_treeToCall()
	{}

	virtual ~CallNode() {}

	virtual void PushState(BehaviourTreeEvaluator& treeEvaluator) const override;

	const Util::StringHash& GetTreeToCall() const { return m_treeToCall; }

private:
	Util::StringHash m_treeToCall;
};
}
