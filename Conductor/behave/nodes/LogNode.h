#pragma once

#include <behave/BehaviourNode.h>

#include <string>

namespace Mem { template <typename T> class UniquePtr; }

namespace Behave
{
namespace Nodes
{
class LogNode : public BehaviourNode
{
public:
	static Mem::UniquePtr<BehaviourNode> LoadFromJSON(const BehaviourNodeFactory& nodeFactory,
		const JSON::JSONObject& jsonObject, const BehaviourTree& tree);

	explicit LogNode(const BehaviourTree& tree)
		: BehaviourNode(tree)
		, m_message()
	{}

	virtual ~LogNode() {}

	virtual void PushState(BehaviourTreeEvaluator& treeEvaluator) const override;

	const char* GetMessage() const { return m_message.c_str(); }

private:
	std::string m_message;
};
}
}
