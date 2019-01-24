#pragma once

#include <asset/AssetHandle.h>
#include <ecs/Component.h>
#include <file/Path.h>
#include <image/Colour.h>
#include <image/Pixel1Image.h>

#include <functional>
#include <string>

namespace Condui
{
/**
 * A TextInputComponent makes an entity able to receive text input and display it. An input handler function is used to
 * process the input as it's received. The default input handler appends all received input to the displayed text and
 * supports backspace.
 */
class TextInputComponent final : public ECS::Component
{
public:
	using InputHandler = std::function<void(TextInputComponent&, const char*)>;

	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Normal;
	static constexpr const char* k_typeName = "text_input_component";
	static const Util::StringHash k_typeHash;

	static bool TryCreateFromFullSerialization(Asset::AssetManager& assetManager,
		const uint8_t*& bytes,
		const uint8_t* bytesEnd,
		const ECS::ComponentID reservedID,
		ECS::ComponentVector& destination);

	static void FullySerialize(const TextInputComponent& component, Collection::Vector<uint8_t>& outBytes);

	static void ApplyFullSerialization(TextInputComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd);

	static void DefaultInputHandler(TextInputComponent& component, const char* text);

public:
	explicit TextInputComponent(const ECS::ComponentID id)
		: ECS::Component(id)
	{}

	virtual ~TextInputComponent() {}

	std::string m_text{};

	InputHandler m_inputHandler{ &DefaultInputHandler };

	Asset::AssetHandle<Image::Pixel1Image> m_codePage{};
	uint16_t m_characterWidthPixels{ 0 };
	uint16_t m_characterHeightPixels{ 0 };

	float m_xScale{ 1.0f };
	float m_yScale{ 1.0f };
	float m_fontScale{ 1.0f };

	Image::ColourARGB m_textColour{ Image::ColoursARBG::k_black };
	Image::ColourARGB m_backgroundColour{ Image::ColoursARBG::k_cyan };
};
}
