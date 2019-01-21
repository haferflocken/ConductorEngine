#pragma once

#include <asset/AssetHandle.h>
#include <ecs/Component.h>
#include <ecs/ComponentInfo.h>
#include <file/Path.h>
#include <image/Colour.h>
#include <image/Pixel1Image.h>

#include <functional>
#include <string>

namespace Condui
{
class TextInputComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr const char* sk_typeName = "text_input_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ComponentInfo> LoadFromJSON(
		Asset::AssetManager& assetManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const override { return sk_typeHash; }

	uint16_t m_characterWidthPixels{ 0 };
	uint16_t m_characterHeightPixels{ 0 };
	File::Path m_codePagePath{};
};

/**
 * A TextInputComponent makes an entity able to receive text input and display it. An input handler function is used to
 * process the input as it's received. The default input handler appends all received input to the displayed text and
 * supports backspace.
 */
class TextInputComponent final : public ECS::Component
{
public:
	using Info = TextInputComponentInfo;
	using InputHandler = std::function<void(TextInputComponent&, const char*)>;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const TextInputComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	static void DefaultInputHandler(TextInputComponent& component, const char* text);

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
