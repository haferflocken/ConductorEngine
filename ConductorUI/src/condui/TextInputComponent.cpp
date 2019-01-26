#include <condui/TextInputComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>
#include <mem/InspectorInfo.h>

namespace Condui
{
const ECS::ComponentType TextInputComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfo TextInputComponent::k_inspectorInfo = MakeInspectorInfo(Condui::TextInputComponent, 0);

void TextInputComponent::FullySerialize(const TextInputComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	// TODO(info) serialize
}

void TextInputComponent::ApplyFullSerialization(Asset::AssetManager& assetManager,
	TextInputComponent& component,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd)
{
	// TODO(info) apply
	//component.m_characterWidthPixels = componentInfo.m_characterWidthPixels;
	//component.m_characterHeightPixels = componentInfo.m_characterHeightPixels;
	//component.m_codePage = assetManager.RequestAsset<Image::Pixel1Image>(componentInfo.m_codePagePath);
}

void TextInputComponent::DefaultInputHandler(TextInputComponent& component, const char* text)
{
	if (strcmp(text, "\b") == 0)
	{
		if (!component.m_text.empty())
		{
			component.m_text.pop_back();
		}
	}
	else
	{
		component.m_text.append(text);
	}
}
}
