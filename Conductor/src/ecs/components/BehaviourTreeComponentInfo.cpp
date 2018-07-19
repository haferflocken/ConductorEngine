#include <ecs/components/BehaviourTreeComponentInfo.h>

#include <behave/BehaviourTreeManager.h>
#include <json/JSONTypes.h>
#include <mem/UniquePtr.h>
#include <util/StringHash.h>

namespace Internal_BehaviourTreeComponentInfo
{
const Util::StringHash k_behaviourTreesArray = Util::CalcHash("behaviour_trees");
}

const Util::StringHash ECS::Components::BehaviourTreeComponentInfo::sk_typeHash =
	Util::CalcHash(BehaviourTreeComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ActorComponentInfo> ECS::Components::BehaviourTreeComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	auto componentInfo = Mem::MakeUnique<BehaviourTreeComponentInfo>();

	const JSON::JSONArray* const behaviourTreesArray =
		jsonObject.FindArray(Internal_BehaviourTreeComponentInfo::k_behaviourTreesArray);
	if (behaviourTreesArray == nullptr)
	{
		Dev::LogWarning("Failed to find behaviour tree array for behaviour_tree_component.");
		return nullptr;
	}

	for (const auto& value : *behaviourTreesArray)
	{
		if (value->GetType() != JSON::ValueType::String)
		{
			Dev::LogWarning("Encountered a non-string element in an actor info behaviour tree array.");
			continue;
		}
		const JSON::JSONString& valueString = static_cast<const JSON::JSONString&>(*value);
		const Behave::BehaviourTree* const behaviourTree = behaviourTreeManager.FindTree(valueString.m_hash);
		if (behaviourTree == nullptr)
		{
			Dev::LogWarning("Failed to find behaviour tree \"%s\".", valueString.m_string.c_str());
			continue;
		}
		componentInfo->m_behaviourTrees.Add(behaviourTree);
	}

	return componentInfo;
}
