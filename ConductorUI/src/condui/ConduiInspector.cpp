#include <condui/ConduiInspector.h>

#include <condui/TextInputComponent.h>
#include <mem/InspectorInfo.h>

#include <array>
#include <charconv>

namespace Internal_ConduiInspector
{
template <typename T>
Condui::TextInputElement::InputHandler MakeIntegerParsingHandler(void* const rawSubject)
{
	T* const subject = static_cast<T*>(rawSubject);
	return [subject](Condui::TextInputComponent& component, const char* text)
	{
		if (strcmp(text, "\r") == 0)
		{
			std::array<char, 32> buffer;
			std::to_chars(buffer.data(), buffer.data() + buffer.size(), *subject);
			component.m_text = buffer.data();

			return;
		}

		Condui::TextInputComponent::DefaultInputHandler(component, text);
		char* const componentTextBegin = &component.m_text.front();
		std::from_chars(componentTextBegin, componentTextBegin + component.m_text.size(), *subject);
	};
}

template <typename T>
Condui::TextInputElement::InputHandler MakeFloatParsingHandler(void* const rawSubject)
{
	T* const subject = static_cast<T*>(rawSubject);
	return [subject](Condui::TextInputComponent& component, const char* text)
	{
		if (strcmp(text, "\r") == 0)
		{
			char buffer[32];
			sprintf_s(buffer, "%f", *subject);
			component.m_text = buffer;

			return;
		}

		Condui::TextInputComponent::DefaultInputHandler(component, text);
		char* const componentTextBegin = &component.m_text.front();
		std::from_chars(componentTextBegin, componentTextBegin + component.m_text.size(), *subject);
	};
}

bool TryMakeInspectorOverrideElement(
	const Mem::InspectorInfo& inspectorInfo,
	void* const rawSubject,
	Condui::ConduiElement& outElement)
{
	using namespace Condui;

	const Mem::InspectorInfoTypeHash i8TypeHash{ typeid(int8_t).hash_code() };
	const Mem::InspectorInfoTypeHash i16TypeHash{ typeid(int16_t).hash_code() };
	const Mem::InspectorInfoTypeHash i32TypeHash{ typeid(int32_t).hash_code() };
	const Mem::InspectorInfoTypeHash i64TypeHash{ typeid(int64_t).hash_code() };

	const Mem::InspectorInfoTypeHash u8TypeHash{ typeid(uint8_t).hash_code() };
	const Mem::InspectorInfoTypeHash u16TypeHash{ typeid(uint16_t).hash_code() };
	const Mem::InspectorInfoTypeHash u32TypeHash{ typeid(uint32_t).hash_code() };
	const Mem::InspectorInfoTypeHash u64TypeHash{ typeid(uint64_t).hash_code() };

	const Mem::InspectorInfoTypeHash f32TypeHash{ typeid(float).hash_code() };
	const Mem::InspectorInfoTypeHash f64TypeHash{ typeid(double).hash_code() };

	const Mem::InspectorInfoTypeHash boolTypeHash{ typeid(bool).hash_code() };

	// Signed integer types.
	if (inspectorInfo.m_typeHash == i8TypeHash)
	{
		outElement = MakeTextInputElement(1.0f, 1.0f, MakeIntegerParsingHandler<int8_t>(rawSubject));
		return true;
	}
	if (inspectorInfo.m_typeHash == i16TypeHash)
	{
		outElement = MakeTextInputElement(1.0f, 1.0f, MakeIntegerParsingHandler<int16_t>(rawSubject));
		return true;
	}
	if (inspectorInfo.m_typeHash == i32TypeHash)
	{
		outElement = MakeTextInputElement(1.0f, 1.0f, MakeIntegerParsingHandler<int32_t>(rawSubject));
		return true;
	}
	if (inspectorInfo.m_typeHash == i64TypeHash)
	{
		outElement = MakeTextInputElement(1.0f, 1.0f, MakeIntegerParsingHandler<int64_t>(rawSubject));
		return true;
	}

	// Unsigned integer types.
	if (inspectorInfo.m_typeHash == u8TypeHash)
	{
		outElement = MakeTextInputElement(1.0f, 1.0f, MakeIntegerParsingHandler<uint8_t>(rawSubject));
		return true;
	}
	if (inspectorInfo.m_typeHash == u16TypeHash)
	{
		outElement = MakeTextInputElement(1.0f, 1.0f, MakeIntegerParsingHandler<uint16_t>(rawSubject));
		return true;
	}
	if (inspectorInfo.m_typeHash == u32TypeHash)
	{
		outElement = MakeTextInputElement(1.0f, 1.0f, MakeIntegerParsingHandler<uint32_t>(rawSubject));
		return true;
	}
	if (inspectorInfo.m_typeHash == u64TypeHash)
	{
		outElement = MakeTextInputElement(1.0f, 1.0f, MakeIntegerParsingHandler<uint64_t>(rawSubject));
		return true;
	}

	// Floating point types.
	if (inspectorInfo.m_typeHash == f32TypeHash)
	{
		outElement = MakeTextInputElement(1.0f, 1.0f, MakeFloatParsingHandler<float>(rawSubject));
		return true;
	}
	if (inspectorInfo.m_typeHash == f64TypeHash)
	{
		outElement = MakeTextInputElement(1.0f, 1.0f, MakeFloatParsingHandler<double>(rawSubject));
		return true;
	}

	// Booleans.
	if (inspectorInfo.m_typeHash == boolTypeHash)
	{
		// TODO(info)
		return false;
	}

	return false;
}

Condui::ConduiElement MakeInspectorElement(
	const Mem::InspectorInfo* const inspectorInfo,
	char* const subject,
	const char* const nameOverride)
{
	using namespace Condui;

	if (inspectorInfo == nullptr)
	{
		const char* const name = (nameOverride != nullptr) ? nameOverride : "UNNAMED";

		char buffer[128];
		sprintf_s(buffer, "%s has no inspector type info!", name);

		return MakeTextDisplayElement(buffer, 1.0f);
	}

	const char* const name = (nameOverride != nullptr) ? nameOverride : inspectorInfo->m_typeName;

	Collection::Vector<Collection::Pair<Math::Matrix4x4, ConduiElement>> childrenWithChildToParentTransforms;
	childrenWithChildToParentTransforms.Emplace(Math::Matrix4x4(), MakeTextDisplayElement(name, 1.0f));

	// Certain types have special overrides for their inspector.
	ConduiElement overrideElement;
	if (TryMakeInspectorOverrideElement(*inspectorInfo, subject, overrideElement))
	{
		Math::Matrix4x4 transform;
		// TODO(info)
		childrenWithChildToParentTransforms.Emplace(transform, std::move(overrideElement));
	}
	// If there's no override, recursively build the inspector with the inspectors of the type's members.
	else
	{
		for (const auto& memberInfo : inspectorInfo->m_memberInfo)
		{
			const Mem::InspectorInfo* const memberInspectorInfo = Mem::InspectorInfo::Find(memberInfo.m_typeHash);
			ConduiElement memberInspectorElement = MakeInspectorElement(
				memberInspectorInfo, subject + memberInfo.m_offset, memberInfo.m_name);

			Math::Matrix4x4 memberTransform;
			// TODO(info)
			childrenWithChildToParentTransforms.Emplace(memberTransform, std::move(memberInspectorElement));
		}
	}

	return MakePanelElement(std::move(childrenWithChildToParentTransforms));
}
}

Condui::ConduiElement Condui::MakeInspectorElement(const Mem::InspectorInfo* const inspectorInfo, void* const subject)
{
	return Internal_ConduiInspector::MakeInspectorElement(inspectorInfo, static_cast<char*>(subject), nullptr);
}
