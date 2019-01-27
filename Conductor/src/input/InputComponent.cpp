#include <input/InputComponent.h>

#include <ecs/ComponentVector.h>
#include <mem/InspectorInfo.h>

namespace Input
{
const ECS::ComponentType InputComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash InputComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Input::InputComponent, 0);

void InputComponent::FullySerialize(const InputComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	// TODO(info) fully serialize
}

void InputComponent::ApplyFullSerialization(
	Asset::AssetManager& assetManager, InputComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	// TODO(info) what inputs does this component care about?
	/*for (const auto& nameHash : componentInfo.m_inputNameHashes)
	{
		inputComponent.m_inputMap[nameHash];
	}*/
}
}
