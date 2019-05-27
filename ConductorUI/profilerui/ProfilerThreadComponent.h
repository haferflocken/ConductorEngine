#pragma once

#include <condui/FontInfo.h>
#include <ecs/Component.h>

namespace ProfilerUI
{
/**
 * An entity with a ProfilerThreadComponent maintains child entities to display a frame-based profile of a thread.
 */
class ProfilerThreadComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Normal;
	static constexpr const char* k_typeName = "profiler_thread_component";
	static const ECS::ComponentType k_type;
	static const Mem::InspectorInfoTypeHash k_inspectorInfoTypeHash;

	static void FullySerialize(const ProfilerThreadComponent& component, Collection::Vector<uint8_t>& outBytes);

	static void ApplyFullSerialization(Asset::AssetManager& assetManager,
		ProfilerThreadComponent& component,
		const uint8_t*& bytes,
		const uint8_t* bytesEnd);

public:
	explicit ProfilerThreadComponent(const ECS::ComponentID id)
		: Component(id)
	{}

	uint64_t m_profilerThreadID{ UINT64_MAX };
	uint64_t m_nextFrameRecordID{ 1 };

	Condui::FontInfo m_fontInfo;
	float m_textHeight;
};
}
