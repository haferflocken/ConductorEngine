#pragma once

#include <asset/AssetHandle.h>
#include <ecs/Component.h>
#include <file/Path.h>
#include <image/Colour.h>
#include <image/Pixel1Image.h>

namespace Condui
{
/**
 * A TextDisplayComponent makes an entity appear in the UI as a text display.
 */
class TextDisplayComponent final : public ECS::Component
{
public:
	using TextUpdateFunction = std::function<void(std::string&)>;

	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Normal;
	static constexpr const char* k_typeName = "text_display_component";
	static const ECS::ComponentType k_type;
	static const Mem::InspectorInfoTypeHash k_inspectorInfoTypeHash;

	static void FullySerialize(const TextDisplayComponent& component, Collection::Vector<uint8_t>& outBytes);

	static void ApplyFullSerialization(Asset::AssetManager& assetManager,
		TextDisplayComponent& component,
		const uint8_t*& bytes,
		const uint8_t* bytesEnd);

public:
	explicit TextDisplayComponent(const ECS::ComponentID id)
		: ECS::Component(id)
	{}

	std::string m_string{};
	TextUpdateFunction m_stringUpdateFunction{};

	Asset::AssetHandle<Image::Pixel1Image> m_codePage{};
	uint16_t m_characterWidthPixels{ 0 };
	uint16_t m_characterHeightPixels{ 0 };

	float m_width{ 1.0f };
	float m_height{ 1.0f };
	float m_textHeight{ 1.0f };

	Image::ColourARGB m_textColour{ Image::ColoursARBG::k_black };
};
}
