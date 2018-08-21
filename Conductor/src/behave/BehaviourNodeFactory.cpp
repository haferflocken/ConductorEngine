#include <behave/BehaviourNodeFactory.h>

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
BehaviourNodeFactory::BehaviourNodeFactory()
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

void BehaviourNodeFactory::RegisterConditionFactoryFunction(
	const char* const conditionType,
	ConditionFactoryFunction fn)
{
	const Util::StringHash conditionTypeHash = Util::CalcHash(conditionType);
	Dev::FatalAssert(m_conditionFactoryFunctions.Find(conditionTypeHash) == m_conditionFactoryFunctions.end(),
		"Attempted to register a factory function for condition type \"%s\", but there already is one.");
	m_conditionFactoryFunctions[conditionTypeHash] = std::move(fn);
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
	// TODO(behave) condition loading
	return nullptr;

	/*const JSON::JSONString* const jsonTypeName = jsonObject.FindString(k_typeKeyHash);
	if (jsonTypeName == nullptr)
	{
		Dev::LogWarning("Failed to find a condition type name.");
		return nullptr;
	}

	const auto factoryItr = m_conditionFactoryFunctions.Find(jsonTypeName->m_hash);
	if (factoryItr == m_conditionFactoryFunctions.end())
	{
		Dev::LogWarning("Failed to find a factory function for condition type \"%s\".", jsonTypeName->m_string.c_str());
		return nullptr;
	}

	return factoryItr->second(jsonObject);*/
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
