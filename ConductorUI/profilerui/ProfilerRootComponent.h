#pragma once

#include <asset/AssetHandle.h>
#include <ecs/Component.h>
#include <image/Colour.h>
#include <image/Pixel1Image.h>

namespace ProfilerUI
{
/**
 * An entity with a ProfilerRootComponent maintains child entities with ProfilerThreadComponents for each thread that
 * is profiled. ProfilerRootComponent is a tag component and is therefore never instantiated.
 */
class ProfilerRootComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Tag;
	static constexpr const char* k_typeName = "profiler_root_component";
	static const ECS::ComponentType k_type;
	static const Mem::InspectorInfoTypeHash k_inspectorInfoTypeHash;

	ProfilerRootComponent() = delete;

	Asset::AssetHandle<Image::Pixel1Image> m_codePage{};
	uint16_t m_characterWidthPixels{ 0 };
	uint16_t m_characterHeightPixels{ 0 };

	float m_width{ 1.0f };
	float m_height{ 1.0f };
	float m_textHeight{ 1.0f };

	Image::ColourARGB m_textColour{ Image::ColoursARBG::k_black };
	Image::ColourARGB m_backgroundColour{ Image::ColoursARBG::k_cyan };
};
}
