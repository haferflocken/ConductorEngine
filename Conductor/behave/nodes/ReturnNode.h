#pragma once

#include <behave/BehaviourNode.h>

namespace Mem { template <typename T> class UniquePtr; }

namespace Behave::Nodes
{
class ReturnNode final : public BehaviourNode
{
public:
	static constexpr const char* k_dslName = "return";

	static Mem::UniquePtr<BehaviourNode> CreateFromNodeExpression(const BehaviourNodeFactory& nodeFactory,
		const AST::Interpreter& interpreter, Parse::NodeExpression& nodeExpression, const BehaviourTree& tree);

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
