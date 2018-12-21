#include <condui/TextDisplayComponent.h>

#include <ecs/ComponentVector.h>

namespace Condui
{
Mem::UniquePtr<ECS::ComponentInfo> TextDisplayComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<TextDisplayComponentInfo>();
}

bool TextDisplayComponent::TryCreateFromInfo(
	Asset::AssetManager& assetManager,
	const TextDisplayComponentInfo& componentInfo,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	destination.Emplace<TextDisplayComponent>(reservedID);
	return true;
}
}
