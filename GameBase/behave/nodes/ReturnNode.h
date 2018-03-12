#pragma once

#include <behave/BehaviourNode.h>

namespace Mem { template <typename T> class UniquePtr; }

namespace Behave
{
namespace Nodes
{
class ReturnNode final : public BehaviourNode
{
public:
	static Mem::UniquePtr<BehaviourNode> LoadFromJSON(const BehaviourNodeFactory& nodeFactory,
		const JSON::JSONObject& jsonObject, const BehaviourTree& tree);

	explicit ReturnNode(const BehaviourTree& tree)
		: BehaviourNode(tree)
		, m_returnsSuccess(true)
	{}

	virtual ~ReturnNode() {}

	virtual void PushState(BehaviourTreeEvaluator& treeEvaluator) const override;

	bool ReturnsSuccess() const { return m_returnsSuccess; }

private:
	bool m_returnsSuccess;
};
}
}
