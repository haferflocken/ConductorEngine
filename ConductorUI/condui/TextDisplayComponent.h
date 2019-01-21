#pragma once

#include <asset/AssetHandle.h>
#include <ecs/Component.h>
#include <ecs/ComponentInfo.h>
#include <file/Path.h>
#include <image/Pixel1Image.h>

namespace Condui
{
class TextDisplayComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr const char* sk_typeName = "text_display_component";
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
 * A TextDisplayComponent makes an entity appear in the UI as a text display.
 */
class TextDisplayComponent final : public ECS::Component
{
public:
	using Info = TextDisplayComponentInfo;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const TextDisplayComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

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
