#include <condui/ConduiInspector.h>

#include <condui/TextInputComponent.h>
#include <math/Matrix4x4.h>
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

		if (component.m_text.empty())
		{
			*subject = 0;
		}
		else
		{
			char* const componentTextBegin = &component.m_text.front();
			std::from_chars(componentTextBegin, componentTextBegin + component.m_text.size(), *subject);
		}
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

		if (component.m_text.empty())
		{
			*subject = 0;
		}
		else
		{
			char* const componentTextBegin = &component.m_text.front();
			std::from_chars(componentTextBegin, componentTextBegin + component.m_text.size(), *subject);
		}
	};
}

Condui::TextInputElement::InputHandler MakeBooleanParsingHandler(void* const rawSubject)
{
	bool* const subject = static_cast<bool*>(rawSubject);
	return [subject](Condui::TextInputComponent& component, const char* text)
	{
		if (strcmp(text, "\r") == 0)
		{
			component.m_text = (*subject) ? "true" : "false";
			return;
		}

		Condui::TextInputComponent::DefaultInputHandler(component, text);

		if (component.m_text.empty())
		{
			*subject = false;
		}
		else
		{
			char* const componentTextBegin = &component.m_text.front();
			if (strncmp(componentTextBegin, "true", 4) == 0)
			{
				*subject = true;
			}
			else
			{
				*subject = false;
			}
		}
	};
}

bool TryMakeInspectorOverrideElement(
	const Mem::InspectorInfo& inspectorInfo,
	void* const rawSubject,
	const float width,
	const float textHeight,
	Condui::ConduiElement& outResult,
	float& outHeight)
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

	const Mem::InspectorInfoTypeHash matrix4x4TypeHash{ typeid(Math::Matrix4x4).hash_code() };

	// Signed integer types.
	if (inspectorInfo.m_typeHash == i8TypeHash)
	{
		outResult = MakeTextInputElement(
			width, textHeight, MakeIntegerParsingHandler<int8_t>(rawSubject), textHeight, Image::ColoursARBG::k_cyan);
		outHeight = textHeight;
		return true;
	}
	if (inspectorInfo.m_typeHash == i16TypeHash)
	{
		outResult = MakeTextInputElement(
			width, textHeight, MakeIntegerParsingHandler<int16_t>(rawSubject), textHeight, Image::ColoursARBG::k_cyan);
		outHeight = textHeight;
		return true;
	}
	if (inspectorInfo.m_typeHash == i32TypeHash)
	{
		outResult = MakeTextInputElement(
			width, textHeight, MakeIntegerParsingHandler<int32_t>(rawSubject), textHeight, Image::ColoursARBG::k_cyan);
		outHeight = textHeight;
		return true;
	}
	if (inspectorInfo.m_typeHash == i64TypeHash)
	{
		outResult = MakeTextInputElement(
			width, textHeight, MakeIntegerParsingHandler<int64_t>(rawSubject), textHeight, Image::ColoursARBG::k_cyan);
		outHeight = textHeight;
		return true;
	}

	// Unsigned integer types.
	if (inspectorInfo.m_typeHash == u8TypeHash)
	{
		outResult = MakeTextInputElement(
			width, textHeight, MakeIntegerParsingHandler<uint8_t>(rawSubject), textHeight, Image::ColoursARBG::k_cyan);
		outHeight = textHeight;
		return true;
	}
	if (inspectorInfo.m_typeHash == u16TypeHash)
	{
		outResult = MakeTextInputElement(
			width, textHeight, MakeIntegerParsingHandler<uint16_t>(rawSubject), textHeight, Image::ColoursARBG::k_cyan);
		outHeight = textHeight;
		return true;
	}
	if (inspectorInfo.m_typeHash == u32TypeHash)
	{
		outResult = MakeTextInputElement(
			width, textHeight, MakeIntegerParsingHandler<uint32_t>(rawSubject), textHeight, Image::ColoursARBG::k_cyan);
		outHeight = textHeight;
		return true;
	}
	if (inspectorInfo.m_typeHash == u64TypeHash)
	{
		outResult = MakeTextInputElement(
			width, textHeight, MakeIntegerParsingHandler<uint64_t>(rawSubject), textHeight, Image::ColoursARBG::k_cyan);
		outHeight = textHeight;
		return true;
	}

	// Floating point types.
	if (inspectorInfo.m_typeHash == f32TypeHash)
	{
		outResult = MakeTextInputElement(
			width, textHeight, MakeFloatParsingHandler<float>(rawSubject), textHeight, Image::ColoursARBG::k_cyan);
		outHeight = textHeight;
		return true;
	}
	if (inspectorInfo.m_typeHash == f64TypeHash)
	{
		outResult = MakeTextInputElement(
			width, textHeight, MakeFloatParsingHandler<double>(rawSubject), textHeight, Image::ColoursARBG::k_cyan);
		outHeight = textHeight;
		return true;
	}

	// Booleans.
	if (inspectorInfo.m_typeHash == boolTypeHash)
	{
		outResult = MakeTextInputElement(
			width, textHeight, MakeBooleanParsingHandler(rawSubject), textHeight, Image::ColoursARBG::k_cyan);
		outHeight = textHeight;
		return true;
	}

	// Math classes.
	if (inspectorInfo.m_typeHash == matrix4x4TypeHash)
	{
		Math::Matrix4x4& subject = *static_cast<Math::Matrix4x4*>(rawSubject);

		Collection::Vector<Collection::Pair<Math::Matrix4x4, Condui::ConduiElement>> matrixSubelements;

		const float subWidth = width * 0.25f;
		const float subHeight = textHeight;
		const Image::ColourARGB primaryColour{ 255, 192, 192, 192 };
		const Image::ColourARGB secondaryColour{ 255, 170, 170, 170 };

		for (size_t i = 0; i < 4; ++i)
		{
			Math::Vector4& column = subject.GetColumn(i);
			
			const float x = i * subWidth;
			const Image::ColourARGB colourA = ((i & 1) == 0) ? primaryColour : secondaryColour;
			const Image::ColourARGB colourB = ((i & 1) == 0) ? secondaryColour : primaryColour;
			
			auto t0 = Math::Matrix4x4::MakeTranslation(x, subHeight * -0.0f, 0.0f);
			auto t1 = Math::Matrix4x4::MakeTranslation(x, subHeight * -1.0f, 0.0f);
			auto t2 = Math::Matrix4x4::MakeTranslation(x, subHeight * -2.0f, 0.0f);
			auto t3 = Math::Matrix4x4::MakeTranslation(x, subHeight * -3.0f, 0.0f);

			auto e0 = MakeTextInputElement(
				subWidth, subHeight, MakeFloatParsingHandler<float>(&column.x), subHeight, colourA);
			auto e1 = MakeTextInputElement(
				subWidth, subHeight, MakeFloatParsingHandler<float>(&column.y), subHeight, colourB);
			auto e2 = MakeTextInputElement(
				subWidth, subHeight, MakeFloatParsingHandler<float>(&column.z), subHeight, colourA);
			auto e3 = MakeTextInputElement(
				subWidth, subHeight, MakeFloatParsingHandler<float>(&column.w), subHeight, colourB);

			matrixSubelements.Emplace(t0, std::move(e0));
			matrixSubelements.Emplace(t1, std::move(e1));
			matrixSubelements.Emplace(t2, std::move(e2));
			matrixSubelements.Emplace(t3, std::move(e3));
		}

		outResult = MakePanelElement(width, textHeight * 4, std::move(matrixSubelements));
		outHeight = textHeight * 4;
		return true;
	}

	return false;
}

void MakeInspectorSubelements(
	const Mem::InspectorInfo* const inspectorInfo,
	char* const subject,
	const char* const nameOverride,
	const float width,
	const float textHeight,
	Collection::Vector<Collection::Pair<Math::Matrix4x4, Condui::ConduiElement>>& subelements,
	float& verticalOffset)
{
	using namespace Condui;

	if (inspectorInfo == nullptr)
	{
		const char* const name = (nameOverride != nullptr) ? nameOverride : "UNNAMED";

		char buffer[128];
		sprintf_s(buffer, "%s has no inspector type info!", name);

		Math::Matrix4x4 titleTransform;
		titleTransform.SetTranslation(0.0f, verticalOffset, 0.0f);
		verticalOffset -= textHeight;

		subelements.Emplace(titleTransform, MakeTextDisplayElement(width, textHeight, buffer, textHeight));
		return;
	}

	const char* const name = (nameOverride != nullptr) ? nameOverride : inspectorInfo->m_typeName;

	Math::Matrix4x4 titleTransform;
	titleTransform.SetTranslation(0.0f, verticalOffset, 0.0f);
	verticalOffset -= textHeight;
	subelements.Emplace(titleTransform, MakeTextDisplayElement(width, textHeight, name, textHeight));

	// Certain types have special overrides for their inspector.
	ConduiElement overrideElement;
	float overrideElementHeight;
	if (TryMakeInspectorOverrideElement(
		*inspectorInfo, subject, width, textHeight, overrideElement, overrideElementHeight))
	{
		Math::Matrix4x4 transform;
		transform.SetTranslation(0.0f, verticalOffset, 0.0f);
		verticalOffset -= overrideElementHeight;
		subelements.Emplace(transform, std::move(overrideElement));
	}
	// If there's no override, recursively build the inspector with the inspectors of the type's members.
	else if (!inspectorInfo->m_memberInfo.IsEmpty())
	{
		for (size_t i = 0, iEnd = inspectorInfo->m_memberInfo.Size(); i < iEnd; ++i)
		{
			const auto& memberInfo = inspectorInfo->m_memberInfo[i];

			const Mem::InspectorInfo* const memberInspectorInfo = Mem::InspectorInfo::Find(memberInfo.m_typeHash);
			MakeInspectorSubelements(
				memberInspectorInfo,
				subject + memberInfo.m_offset,
				memberInfo.m_name,
				width,
				textHeight,
				subelements,
				verticalOffset);
		}
	}
}
}

Condui::ConduiElement Condui::MakeInspectorElement(
	const Mem::InspectorInfo* const inspectorInfo,
	void* const subject,
	const float width,
	const float height,
	const float textHeight)
{
	Collection::Vector<Collection::Pair<Math::Matrix4x4, ConduiElement>> subelements;
	float verticalOffset = 0.0f;
	Internal_ConduiInspector::MakeInspectorSubelements(
		inspectorInfo,
		static_cast<char*>(subject),
		nullptr,
		width,
		textHeight,
		subelements,
		verticalOffset);

	return Condui::MakePanelElement(width, height, std::move(subelements));
}
