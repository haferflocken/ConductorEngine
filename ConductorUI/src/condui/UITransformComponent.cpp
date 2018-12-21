#include <condui/UITransfromComponent.h>

#include <ecs/ComponentVector.h>

namespace Condui
{
Mem::UniquePtr<ECS::ComponentInfo> UITransformComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<UITransformComponentInfo>();
}

bool UITransformComponent::TryCreateFromInfo(
	Asset::AssetManager& assetManager,
	const UITransformComponentInfo& componentInfo,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	destination.Emplace<UITransformComponent>(reservedID);
	return true;
}
}
