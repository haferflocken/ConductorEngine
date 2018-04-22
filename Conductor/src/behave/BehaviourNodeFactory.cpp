#include <behave/BehaviourNodeFactory.h>

#include <behave/BehaviourCondition.h>
#include <behave/BehaviourNode.h>
#include <behave/nodes/CallNode.h>
#include <behave/nodes/ConditionalNode.h>
#include <behave/nodes/DomainNode.h>
#include <behave/nodes/LogNode.h>
#include <behave/nodes/ReturnNode.h>
#include <behave/nodes/SelectorNode.h>
#include <behave/nodes/SequenceNode.h>

#include <dev/Dev.h>
#include <json/JSONTypes.h>

#include <algorithm>
#include <string>

using namespace Behave;

namespace
{
const Util::StringHash k_typeKeyHash = Util::CalcHash("type");
}

BehaviourNodeFactory::BehaviourNodeFactory()
{
	RegisterNodeFactoryFunction("call", &Nodes::CallNode::LoadFromJSON);
	RegisterNodeFactoryFunction("conditional", &Nodes::ConditionalNode::LoadFromJSON);
	RegisterNodeFactoryFunction("domain", &Nodes::DomainNode::LoadFromJSON);
	RegisterNodeFactoryFunction("log", &Nodes::LogNode::LoadFromJSON);
	RegisterNodeFactoryFunction("return", &Nodes::ReturnNode::LoadFromJSON);
	RegisterNodeFactoryFunction("selector", &Nodes::SelectorNode::LoadFromJSON);
	RegisterNodeFactoryFunction("sequence", &Nodes::SequenceNode::LoadFromJSON);
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
	const JSON::JSONObject& jsonObject,
	const BehaviourTree& tree) const
{
	const JSON::JSONString* const jsonTypeName = jsonObject.FindString(k_typeKeyHash);
	if (jsonTypeName == nullptr)
	{
		Dev::LogWarning("Failed to find a node type name.");
		return nullptr;
	}

	const auto factoryItr = m_nodeFactoryFunctions.Find(jsonTypeName->m_hash);
	if (factoryItr == m_nodeFactoryFunctions.end())
	{
		for (const auto& entry : m_nodeFactoryFunctions)
		{
			Dev::Log("%s", Util::ReverseHash(entry.first));
		}
		Dev::LogWarning("Failed to find a factory function for node type \"%s\".", jsonTypeName->m_string.c_str());
		return nullptr;
	}

	return factoryItr->second(*this, jsonObject, tree);
}

Mem::UniquePtr<BehaviourCondition> BehaviourNodeFactory::MakeCondition(const JSON::JSONObject& jsonObject) const
{
	const JSON::JSONString* const jsonTypeName = jsonObject.FindString(k_typeKeyHash);
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

	return factoryItr->second(jsonObject);
}

bool BehaviourNodeFactory::TryMakeNodesFrom(
	const JSON::JSONObject& jsonObject,
	const BehaviourTree& tree,
	const Util::StringHash nodesKeyHash,
	Collection::Vector<Mem::UniquePtr<BehaviourNode>>& outNodes) const
{
	const JSON::JSONArray* const nodesArray = jsonObject.FindArray(nodesKeyHash);
	if (nodesArray == nullptr)
	{
		Dev::LogWarning("Failed to find an array at \"%s\".", Util::ReverseHash(nodesKeyHash));
		return false;
	}

	for (const auto& value : *nodesArray)
	{
		if (value->GetType() != JSON::ValueType::Object)
		{
			Dev::LogWarning("Encountered a non-object value within \"%s\".", Util::ReverseHash(nodesKeyHash));
			return false;
		}
		Mem::UniquePtr<BehaviourNode> node = MakeNode(*static_cast<const JSON::JSONObject*>(value.Get()), tree);
		if (node == nullptr)
		{
			Dev::LogWarning("Failed to make node within \"%s\".", Util::ReverseHash(nodesKeyHash));
			return false;
		}
		outNodes.Add(std::move(node));
	}

	return true;
}
