#include <condui/TextInputComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>

namespace Condui
{
const Util::StringHash TextInputComponentInfo::sk_typeHash = Util::CalcHash(TextInputComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> TextInputComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<TextInputComponentInfo>();
}

bool TextInputComponent::TryCreateFromInfo(
	Asset::AssetManager& assetManager,
	const TextInputComponentInfo& componentInfo,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	TextInputComponent& component = destination.Emplace<TextInputComponent>(reservedID);
	component.m_characterWidthPixels = componentInfo.m_characterWidthPixels;
	component.m_characterHeightPixels = componentInfo.m_characterHeightPixels;
	component.m_codePage = assetManager.RequestAsset<Image::Pixel1Image>(componentInfo.m_codePagePath);
	return true;
}
}
