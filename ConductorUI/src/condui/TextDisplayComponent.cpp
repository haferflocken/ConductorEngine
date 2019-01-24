#include <condui/TextDisplayComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>

namespace Condui
{
const Util::StringHash TextDisplayComponent::k_typeHash = Util::CalcHash(k_typeName);

bool TextDisplayComponent::TryCreateFromFullSerialization(Asset::AssetManager& assetManager,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	TextDisplayComponent& component = destination.Emplace<TextDisplayComponent>(reservedID);
	//component.m_codePage = assetManager.RequestAsset<Image::Pixel1Image>(componentInfo.m_codePagePath);
	//component.m_characterWidthPixels = componentInfo.m_characterWidthPixels;
	//component.m_characterHeightPixels = componentInfo.m_characterHeightPixels;
	return true;
}
}
