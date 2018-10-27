#include <input/InputComponent.h>

#include <ecs/ComponentVector.h>

namespace Input
{
const Util::StringHash InputComponentInfo::sk_typeHash = Util::CalcHash(sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> InputComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<InputComponentInfo>();
}

bool InputComponent::TryCreateFromInfo(Asset::AssetManager& assetManager, const InputComponentInfo& componentInfo,
	const ECS::ComponentID reservedID, ECS::ComponentVector& destination)
{
	destination.Emplace<InputComponent>(reservedID);
	return true;
}
}
