#include <condui/TextDisplayComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>
#include <mem/InspectorInfo.h>

namespace Condui
{
const ECS::ComponentType TextDisplayComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfo TextDisplayComponent::k_inspectorInfo = MakeInspectorInfo(Condui::TextDisplayComponent, 0);

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
