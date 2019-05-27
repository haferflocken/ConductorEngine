#include <profilerui/ProfilerRootComponent.h>

#include <mem/InspectorInfo.h>

namespace ProfilerUI
{
const ECS::ComponentType ProfilerRootComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash ProfilerRootComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(ProfilerUI::ProfilerRootComponent, 0);

void ProfilerRootComponent::FullySerialize(const ProfilerRootComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	AMP_LOG_ERROR("UI components shouldn't be serialized!");
}

void ProfilerRootComponent::ApplyFullSerialization(Asset::AssetManager& assetManager,
	ProfilerRootComponent& component,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd)
{
	AMP_LOG_ERROR("UI components shouldn't be deserialized!");
}
}
