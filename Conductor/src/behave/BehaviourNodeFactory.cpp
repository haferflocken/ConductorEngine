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
	AST::ExpressionCompileResult compileResult = m_interpreter.Compile(expression);
	if (!compileResult.Is<AST::Expression>())
	{
		const AST::TypeCheckFailure& typeCheckFailure = compileResult.Get<AST::TypeCheckFailure>();
		Dev::LogWarning("Type Checking Failure: %s", typeCheckFailure.m_message.c_str());
		return nullptr;
	}

	AST::Expression& compiledExpression = compileResult.Get<AST::Expression>();

	bool expressionResultsInBool = false;
	compiledExpression.Match(
		[&](const bool&) { expressionResultsInBool = true; },
		[](const double&) {},
		[](const std::string&) {},
		[](const ECS::ComponentType&) {},
		[](const AST::TreeIdentifier&) {},
		[&](const AST::FunctionCallExpression& functionCallExpression)
		{
			expressionResultsInBool =
				(functionCallExpression.m_boundFunction.GetReturnType() == AST::ExpressionResultTypes::Boolean);
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
		if (!expression.Is<Parse::NodeExpression>())
		{
			Dev::LogWarning("Cannot create a node from an expression that is not a node expression.");
			return false;
		}

		Mem::UniquePtr<BehaviourNode> node = MakeNode(expression.Get<Parse::NodeExpression>(), tree);
		if (node == nullptr)
		{
			return false;
		}
		outNodes.Add(std::move(node));
	}

	return true;
}
}
