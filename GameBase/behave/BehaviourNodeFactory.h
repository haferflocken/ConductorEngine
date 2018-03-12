#pragma once

#include <collection/Vector.h>
#include <mem/UniquePtr.h>
#include <util/StringHash.h>

#include <unordered_map>

namespace JSON
{
class JSONObject;
}

namespace Behave
{
class BehaviourCondition;
class BehaviourNode;
class BehaviourTree;

/**
 * Creates behaviour nodes from JSON files.
 */
class BehaviourNodeFactory
{
public:
	using NodeFactoryFunction =
		Mem::UniquePtr<BehaviourNode>(*)(const BehaviourNodeFactory&, const JSON::JSONObject&, const BehaviourTree&);
	using ConditionFactoryFunction =
		Mem::UniquePtr<BehaviourCondition>(*)(const JSON::JSONObject&);

	BehaviourNodeFactory();

	void RegisterNodeFactoryFunction(const char* const nodeType, NodeFactoryFunction fn);
	void RegisterConditionFactoryFunction(const char* const conditionType, ConditionFactoryFunction fn);

	Mem::UniquePtr<BehaviourNode> MakeNode(const JSON::JSONObject& jsonObject, const BehaviourTree& tree) const;
	Mem::UniquePtr<BehaviourCondition> MakeCondition(const JSON::JSONObject& jsonObject) const;

	bool TryMakeNodesFrom(const JSON::JSONObject& object, const BehaviourTree& tree,
		const Util::StringHash nodesKeyHash, Collection::Vector<Mem::UniquePtr<BehaviourNode>>& outNodes) const;

private:
	// Maps hashes of node type names to factory functions for those node types.
	std::unordered_map<Util::StringHash, NodeFactoryFunction> m_nodeFactoryFunctions;

	// Maps hashes of condition type names to factory functions for those condition types.
	std::unordered_map<Util::StringHash, ConditionFactoryFunction> m_conditionFactoryFunctions;
};
}
