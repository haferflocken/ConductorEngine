#pragma once

#include <asset/AssetHandle.h>
#include <ecs/Component.h>
#include <file/Path.h>
#include <image/Pixel1Image.h>

namespace Condui
{
/**
 * A TextDisplayComponent makes an entity appear in the UI as a text display.
 */
class TextDisplayComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Normal;
	static constexpr const char* k_typeName = "text_display_component";
	static const Util::StringHash k_typeHash;

	static bool TryCreateFromFullSerialization(Asset::AssetManager& assetManager,
		const uint8_t*& bytes,
		const uint8_t* bytesEnd,
		const ECS::ComponentID reservedID,
		ECS::ComponentVector& destination);

	static void FullySerialize(const TextDisplayComponent& component, Collection::Vector<uint8_t>& outBytes);

	static void ApplyFullSerialization(TextDisplayComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd);

public:
	explicit TextDisplayComponent(const ECS::ComponentID id)
		: ECS::Component(id)
	{}

	virtual ~TextDisplayComponent() {}

	std::string m_string{};

	Asset::AssetHandle<Image::Pixel1Image> m_codePage{};
	uint16_t m_characterWidthPixels{ 0 };
	uint16_t m_characterHeightPixels{ 0 };

	float m_fontScale{ 1.0f };
};
}
