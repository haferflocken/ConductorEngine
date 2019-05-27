#pragma once

#include <asset/AssetHandle.h>
#include <condui/FontInfo.h>
#include <ecs/Component.h>

namespace ProfilerUI
{
/**
 * An entity with a ProfilerRootComponent maintains child entities with ProfilerThreadComponents for each thread that
 * is profiled.
 */
class ProfilerRootComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Normal;
	static constexpr const char* k_typeName = "profiler_root_component";
	static const ECS::ComponentType k_type;
	static const Mem::InspectorInfoTypeHash k_inspectorInfoTypeHash;

	static void FullySerialize(const ProfilerRootComponent& component, Collection::Vector<uint8_t>& outBytes);

	static void ApplyFullSerialization(Asset::AssetManager& assetManager,
		ProfilerRootComponent& component,
		const uint8_t*& bytes,
		const uint8_t* bytesEnd);

public:
	explicit ProfilerRootComponent(const ECS::ComponentID id)
		: Component(id)
	{}

	Condui::FontInfo m_fontInfo;
	Image::ColourARGB m_backgroundColour{ Image::ColoursARBG::k_cyan };

	float m_width{ 1.0f };
	float m_height{ 1.0f };
	float m_textHeight{ 1.0f };
};
}
