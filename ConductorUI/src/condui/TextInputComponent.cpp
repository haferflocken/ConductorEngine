#include <condui/TextInputComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>

namespace Condui
{
const Util::StringHash TextInputComponent::k_typeHash = Util::CalcHash(k_typeName);

bool TextInputComponent::TryCreateFromFullSerialization(Asset::AssetManager& assetManager,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	TextInputComponent& component = destination.Emplace<TextInputComponent>(reservedID);
	//component.m_characterWidthPixels = componentInfo.m_characterWidthPixels;
	//component.m_characterHeightPixels = componentInfo.m_characterHeightPixels;
	//component.m_codePage = assetManager.RequestAsset<Image::Pixel1Image>(componentInfo.m_codePagePath);
	return true;
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
