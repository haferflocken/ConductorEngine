#include <condui/components/TextDisplayComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>
#include <mem/InspectorInfo.h>

namespace Condui
{
const ECS::ComponentType TextDisplayComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash TextDisplayComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Condui::TextDisplayComponent, 0);

void TextDisplayComponent::FullySerialize(const TextDisplayComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	AMP_LOG_ERROR("UI components shouldn't be serialized!");
}

void TextDisplayComponent::ApplyFullSerialization(Asset::AssetManager& assetManager,
	TextDisplayComponent& component,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd)
{
	AMP_LOG_ERROR("UI components shouldn't be deserialized!");
}
}
