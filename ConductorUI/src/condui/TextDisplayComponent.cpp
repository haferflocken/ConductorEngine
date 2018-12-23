#include <condui/TextDisplayComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>

namespace Condui
{
const Util::StringHash TextDisplayComponentInfo::sk_typeHash = Util::CalcHash(TextDisplayComponentInfo::sk_typeName);

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
	TextDisplayComponent& component = destination.Emplace<TextDisplayComponent>(reservedID);
	component.m_characterWidthPixels = componentInfo.m_characterWidthPixels;
	component.m_characterHeightPixels = componentInfo.m_characterHeightPixels;
	component.m_codePage = assetManager.RequestAsset<Image::Pixel1Image>(componentInfo.m_codePagePath);
	return true;
}
}
