#include <input/InputComponent.h>

#include <ecs/ComponentVector.h>
#include <json/JSONTypes.h>

namespace Input
{
const Util::StringHash InputComponent::k_typeHash = Util::CalcHash(k_typeName);

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
