#include <condui/TextDisplayComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>

namespace Condui
{
const Util::StringHash TextDisplayComponent::k_typeHash = Util::CalcHash(k_typeName);

void TextDisplayComponent::FullySerialize(const TextDisplayComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	// TODO(info) serialize
}

void TextDisplayComponent::ApplyFullSerialization(Asset::AssetManager& assetManager,
	TextDisplayComponent& component,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd)
{
	// TODO(info) apply
	//component.m_codePage = assetManager.RequestAsset<Image::Pixel1Image>(componentInfo.m_codePagePath);
	//component.m_characterWidthPixels = componentInfo.m_characterWidthPixels;
	//component.m_characterHeightPixels = componentInfo.m_characterHeightPixels;
}
}
