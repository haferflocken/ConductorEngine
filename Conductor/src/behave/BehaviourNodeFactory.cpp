#include <behave/BehaviourNodeFactory.h>

#include <behave/ast/Interpreter.h>
#include <behave/BehaviourCondition.h>
#include <behave/BehaviourNode.h>
#include <behave/parse/BehaveParsedTree.h>
#include <behave/nodes/CallNode.h>
#include <behave/nodes/ConditionalNode.h>
#include <behave/nodes/DomainNode.h>
#include <behave/nodes/LogNode.h>
#include <behave/nodes/ReturnNode.h>
#include <behave/nodes/SelectorNode.h>
#include <behave/nodes/SequenceNode.h>

#include <dev/Dev.h>

#include <algorithm>
#include <string>

namespace Behave
{
BehaviourNodeFactory::BehaviourNodeFactory(const AST::Interpreter& interpreter)
	: m_interpreter(interpreter)
	, m_nodeFactoryFunctions()
{
	RegisterNodeType<Nodes::CallNode>();
	RegisterNodeType<Nodes::DomainNode>();
	RegisterNodeType<Nodes::LogNode>();
	RegisterNodeType<Nodes::ReturnNode>();
	RegisterNodeType<Nodes::SelectorNode>();
	RegisterNodeType<Nodes::SequenceNode>();
}

void BehaviourNodeFactory::RegisterNodeFactoryFunction(
	const char* const nodeType,
	NodeFactoryFunction fn)
{
	const Util::StringHash nodeTypeHash = Util::CalcHash(nodeType);
	Dev::FatalAssert(m_nodeFactoryFunctions.Find(nodeTypeHash) == m_nodeFactoryFunctions.end(),
		"Attempted to register a factory function for node type \"%s\", but there already is one.");
	m_nodeFactoryFunctions[nodeTypeHash] = std::move(fn);
}

Mem::UniquePtr<BehaviourNode> BehaviourNodeFactory::MakeNode(
	const Parse::NodeExpression& nodeExpression,
	const BehaviourTree& tree) const
{
	const auto factoryItr = m_nodeFactoryFunctions.Find(Util::CalcHash(nodeExpression.m_nodeName));
	if (factoryItr == m_nodeFactoryFunctions.end())
	{
		for (const auto& entry : m_nodeFactoryFunctions)
		{
			Dev::Log("%s", Util::ReverseHash(entry.first));
		}
		Dev::LogWarning("Failed to find a factory function for node type \"%s\".", nodeExpression.m_nodeName.c_str());
		return nullptr;
	}

	return factoryItr->second(*this, nodeExpression, tree);
}

Mem::UniquePtr<BehaviourCondition> BehaviourNodeFactory::MakeCondition(const Parse::Expression& expression) const
{
	AST::Expression compiledExpression = m_interpreter.Compile(expression);
	if (!compiledExpression.m_variant.IsAny())
	{
		return nullptr;
	}

	bool expressionResultsInBool = false;
	compiledExpression.m_variant.Match(
		[&](const AST::BooleanLiteralExpression&) { expressionResultsInBool = true; },
		[](const AST::NumericLiteralExpression&) {},
		[](const AST::ComponentTypeLiteralExpression&) {},
		[](const AST::TreeIdentifierExpression&) {},
		[&](const AST::FunctionCallExpression& functionCallExpression)
		{
			// TODO(behave) type check the function call!
			expressionResultsInBool = true;
		});

	if (!expressionResultsInBool)
	{
		Dev::LogWarning("Conditions may only be constructed from expressions that result in bool.");
		return nullptr;
	}

	return Mem::MakeUnique<BehaviourCondition>(std::move(compiledExpression));
}

bool BehaviourNodeFactory::TryMakeNodesFrom(
	const Collection::Vector<Parse::Expression>& expressions,
	const BehaviourTree& tree,
	Collection::Vector<Mem::UniquePtr<BehaviourNode>>& outNodes) const
{
	for (const auto& expression : expressions)
	{
		if (!expression.m_variant.Is<Parse::NodeExpression>())
		{
			Dev::LogWarning("Cannot create a node from an expression that is not a node expression.");
			return false;
		}

		Mem::UniquePtr<BehaviourNode> node = MakeNode(expression.m_variant.Get<Parse::NodeExpression>(), tree);
		if (node == nullptr)
		{
			return false;
		}
		outNodes.Add(std::move(node));
	}

	return true;
}
}
