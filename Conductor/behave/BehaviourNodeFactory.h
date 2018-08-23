#pragma once

#include <collection/Vector.h>
#include <collection/VectorMap.h>
#include <mem/UniquePtr.h>
#include <util/StringHash.h>

namespace Behave
{
class BehaviourCondition;
class BehaviourNode;
class BehaviourTree;

namespace AST
{
class Interpreter;
}

namespace Parse
{
struct Expression;
struct NodeExpression;
}

/**
 * Creates behaviour nodes from JSON files.
 */
class BehaviourNodeFactory
{
public:
	using NodeFactoryFunction =
		Mem::UniquePtr<BehaviourNode>(*)(const BehaviourNodeFactory&, const Parse::NodeExpression&, const BehaviourTree&);

	explicit BehaviourNodeFactory(const AST::Interpreter& interpreter);

	template <typename NodeType>
	void RegisterNodeType();

	Mem::UniquePtr<BehaviourNode> MakeNode(const Parse::NodeExpression& nodeExpression, const BehaviourTree& tree) const;
	Mem::UniquePtr<BehaviourCondition> MakeCondition(const Parse::Expression& expression) const;

	bool TryMakeNodesFrom(const Collection::Vector<Parse::Expression>& expressions, const BehaviourTree& tree,
		Collection::Vector<Mem::UniquePtr<BehaviourNode>>& outNodes) const;

private:
	void RegisterNodeFactoryFunction(const char* const nodeType, NodeFactoryFunction fn);

	const AST::Interpreter& m_interpreter;

	// Maps hashes of node type names to factory functions for those node types.
	Collection::VectorMap<Util::StringHash, NodeFactoryFunction> m_nodeFactoryFunctions;
};

template <typename NodeType>
void BehaviourNodeFactory::RegisterNodeType()
{
	RegisterNodeFactoryFunction(NodeType::k_dslName, &NodeType::CreateFromNodeExpression);
}
}
