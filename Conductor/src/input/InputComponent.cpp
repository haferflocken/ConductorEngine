#include <input/InputComponent.h>

#include <ecs/ComponentVector.h>
#include <mem/DeserializeLittleEndian.h>
#include <mem/InspectorInfo.h>
#include <mem/SerializeLittleEndian.h>

namespace Input
{
const ECS::ComponentType InputComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash InputComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Input::InputComponent, 0);

void InputComponent::FullySerialize(const InputComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	// Only the client ID and desired inputs are saved.
	Mem::LittleEndian::Serialize(component.m_clientID.GetN(), outBytes);

	Mem::LittleEndian::Serialize(component.m_inputMap.Size(), outBytes);
	for (const auto& entry : component.m_inputMap)
	{
		const char* const inputName = Util::ReverseHash(entry.first);
		Mem::LittleEndian::Serialize(inputName, outBytes);
	}
}

void InputComponent::ApplyFullSerialization(
	Asset::AssetManager& assetManager, InputComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	const auto maybeClientID = Mem::LittleEndian::DeserializeUi16(bytes, bytesEnd);
	if (!maybeClientID.second)
	{
		return;
	}

	const auto maybeNumInputs = Mem::LittleEndian::DeserializeUi32(bytes, bytesEnd);
	if (!maybeNumInputs.second)
	{
		return;
	}
	for (size_t i = 0; i < maybeNumInputs.first; ++i)
	{
		char inputNameBuffer[64];
		if (!Mem::LittleEndian::DeserializeString(bytes, bytesEnd, inputNameBuffer))
		{
			return;
		}
		const Util::StringHash inputNameHash = Util::CalcHash(inputNameBuffer);
		component.m_inputMap[inputNameHash];
	}
}
}
