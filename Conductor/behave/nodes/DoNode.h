#pragma once

#include <behave/BehaviourNode.h>
#include <collection/Vector.h>
#include <mem/UniquePtr.h>

namespace Behave
{
namespace AST { struct Expression; }

namespace Nodes
{
/**
 * Executes a series of Behave DSL expressions so that simple functions can just be bound to the interpreter
 * rather than encapsulated in their own node.
 */
class DoNode final : public BehaviourNode
{
public:
	static constexpr const char* k_dslName = "do";

	static Mem::UniquePtr<BehaviourNode> CreateFromNodeExpression(const BehaviourNodeFactory& nodeFactory,
		const AST::Interpreter& interpreter, Parse::NodeExpression& nodeExpression, const BehaviourTree& tree);

	DoNode(const BehaviourTree& tree, Collection::Vector<AST::Expression>&& expressions);
	virtual ~DoNode();

	virtual void PushState(BehaviourTreeEvaluator& treeEvaluator) const override;

	const Collection::Vector<AST::Expression>& GetExpressions() const { return m_expressions; }

private:
	Collection::Vector<AST::Expression> m_expressions;
};
}
}
