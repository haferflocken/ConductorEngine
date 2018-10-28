#include <input/InputComponent.h>

#include <ecs/ComponentVector.h>
#include <json/JSONTypes.h>

namespace Input
{
namespace Internal_InputComponent
{
const Util::StringHash k_inputNamesHash = Util::CalcHash("input_names");
}

const Util::StringHash InputComponentInfo::sk_typeHash = Util::CalcHash(sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> InputComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	auto outComponentInfo = Mem::MakeUnique<InputComponentInfo>();
	
	const JSON::JSONArray* const maybeInputNames = jsonObject.FindArray(Internal_InputComponent::k_inputNamesHash);
	if (maybeInputNames != nullptr)
	{
		for (const auto& jsonValue : *maybeInputNames)
		{
			if (jsonValue->GetType() == JSON::ValueType::String)
			{
				const JSON::JSONString& inputName = static_cast<const JSON::JSONString&>(*jsonValue);
				outComponentInfo->m_inputNameHashes.Add(inputName.m_hash);
			}
		}
	}

	return outComponentInfo;
}

bool InputComponent::TryCreateFromInfo(Asset::AssetManager& assetManager, const InputComponentInfo& componentInfo,
	const ECS::ComponentID reservedID, ECS::ComponentVector& destination)
{
	InputComponent& inputComponent = destination.Emplace<InputComponent>(reservedID);
	for (const auto& nameHash : componentInfo.m_inputNameHashes)
	{
		inputComponent.m_inputMap[nameHash];
	}
	return true;
}
}
