#include <condui/components/TextInputComponent.h>

#include <mem/InspectorInfo.h>

namespace Condui
{
const ECS::ComponentType TextInputComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash TextInputComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Condui::TextInputComponent, 0);

void TextInputComponent::FullySerialize(const TextInputComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	AMP_LOG_ERROR("UI components shouldn't be serialized!");
}

void TextInputComponent::ApplyFullSerialization(Asset::AssetManager& assetManager,
	TextInputComponent& component,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd)
{
	AMP_LOG_ERROR("UI components shouldn't be deserialized!");
}

void TextInputComponent::DefaultInputHandler(TextInputComponent& component, const char* text)
{
	if (strcmp(text, "\b") == 0)
	{
		if (!component.m_text.empty())
		{
			component.m_text.pop_back();
		}
	}
	else
	{
		component.m_text.append(text);
	}
}
}
