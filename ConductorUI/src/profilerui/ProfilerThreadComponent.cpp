#include <profilerui/ProfilerThreadComponent.h>

#include <mem/InspectorInfo.h>

namespace ProfilerUI
{
const ECS::ComponentType ProfilerThreadComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash ProfilerThreadComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(ProfilerUI::ProfilerThreadComponent, 0);

void ProfilerThreadComponent::FullySerialize(const ProfilerThreadComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	AMP_LOG_ERROR("UI components shouldn't be serialized!");
}

void ProfilerThreadComponent::ApplyFullSerialization(Asset::AssetManager& assetManager,
	ProfilerThreadComponent& component,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd)
{
	AMP_LOG_ERROR("UI components shouldn't be deserialized!");
}
}
