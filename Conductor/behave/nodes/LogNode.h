#pragma once

#include <behave/BehaviourNode.h>

#include <string>

namespace Mem { template <typename T> class UniquePtr; }

namespace Behave::Nodes
{
class LogNode : public BehaviourNode
{
public:
	static constexpr const char* k_dslName = "log";

	static Mem::UniquePtr<BehaviourNode> CreateFromNodeExpression(const BehaviourNodeFactory& nodeFactory,
		const AST::Interpreter& interpreter, const Parse::NodeExpression& nodeExpression, const BehaviourTree& tree);

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
