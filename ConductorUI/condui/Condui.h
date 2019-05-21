#pragma once

#include <asset/AssetHandle.h>
#include <collection/Pair.h>
#include <collection/Variant.h>
#include <collection/Vector.h>
#include <collection/VectorMap.h>
#include <image/Colour.h>
#include <image/Pixel1Image.h>
#include <math/Matrix4x4.h>
#include <mem/UniquePtr.h>
#include <util/StringHash.h>

#include <functional>
#include <string>

namespace ECS
{
class Entity;
class EntityManager;
}

namespace Condui
{
struct ConduiElement;
class TextInputComponent;

/**
 * An element that displays a string. The string can be updated by a function.
 */
struct TextDisplayElement final
{
	using TextUpdateFunction = std::function<void(std::string&)>;

	std::string m_initialString{};

	// The function called to update this element's string.
	TextUpdateFunction m_updateFunction;

	// The bounds of the text display rectangle.
	float m_width{ 1.0f };
	float m_height{ 1.0f };
	float m_textHeight{ 1.0f };
};

/**
 * An element that can be typed in.
 */
struct TextInputElement final
{
	using InputHandler = std::function<void(TextInputComponent&, const char*)>;

	// The function called to process this element's input.
	InputHandler m_inputHandler{};

	// The bounds of the text input rectangle.
	float m_width{ 1.0f };
	float m_height{ 1.0f };
	float m_textHeight{ 1.0f };

	// The colours of the input rectangle.
	Image::ColourARGB m_backgroundColour{ Image::ColoursARBG::k_cyan };
};

/**
 * An element that contains other elements transformed from itself.
 */
struct PanelElement final
{
	float m_width{ 1.0f };
	float m_height{ 1.0f };

	Collection::Vector<Math::Matrix4x4> m_childRelativeTransforms;
	Collection::Vector<ConduiElement> m_children;
};

/**
 * An element that contains other elements which it stacks vertically. Adjusts to changes in element height.
 */
struct StackingPanelElement final
{
	float m_width{ 1.0f };
	float m_height{ 0.0f };

	Collection::Vector<ConduiElement> m_children;
};

/**
 * An element with one child that is enabled/disabled by an input.
 */
struct InputToggledElement final
{
	Mem::UniquePtr<ConduiElement> m_child;
	// The input which toggles the child element.
	Util::StringHash m_inputName;
	// When the input value increases above or equal to m_inputThresholdUp, the child is toggled.
	// It will not toggle crossing m_inputThresholdUp again until the input decreases below m_inputThresoldDown.
	float m_inputThresholdUp;
	float m_inputThresholdDown;
};

/**
 * A union of all element types.
 */
struct ConduiElement final : public Collection::Variant<
	TextDisplayElement,
	TextInputElement,
	PanelElement,
	StackingPanelElement>
{
public:
	using Variant::Variant;

	template <typename T, typename... Args>
	static ConduiElement Make(Args&&... args)
	{
		return ConduiElement(Variant::Make<T, Args...>(std::forward<Args>(args)...));
	}

public:
	ConduiElement() = default;

	ConduiElement(Variant&& v)
		: Variant(std::move(v))
	{}

	float GetWidth() const;
	float GetHeight() const;
};

/**
 * Functions to create Condui elements in a declarative style.
 */
ConduiElement MakeTextDisplayElement(
	const float width, const float height, const char* const str, const float textHeight);
ConduiElement MakeTextDisplayElement(
	const float width,
	const float height,
	const char* const str,
	TextDisplayElement::TextUpdateFunction&& updateFunction,
	const float textHeight);
ConduiElement MakeTextInputElement(
	const float width,
	const float height,
	TextInputElement::InputHandler&& inputHandler,
	const float textHeight,
	const Image::ColourARGB backgroundColour);
ConduiElement MakePanelElement(
	const float width,
	const float height,
	Collection::Vector<Collection::Pair<Math::Matrix4x4, ConduiElement>>&& childrenWithRelativeTransforms);
ConduiElement MakeStackingPanelElement(
	const float width,
	Collection::Vector<ConduiElement>&& children);

/**
 * Functions to make Condui elements for specific purposes.
 */
ConduiElement MakeTextInputCommandElement(
	const float width,
	const float height,
	Collection::VectorMap<const char*, std::function<void(TextInputComponent&)>>&& commandMap,
	const Image::ColourARGB backgroundColour);

/**
 * Functions to actualize a ConduiElement as an ECS::Entity. These consume the ConduiElement.
 */
struct FontInfo
{
	Asset::AssetHandle<Image::Pixel1Image> m_codePage;
	uint16_t m_characterWidthPixels;
	uint16_t m_characterHeightPixels;
	Image::ColourARGB m_textColour;
};
ECS::Entity& CreateConduiEntity(ECS::EntityManager& entityManager, ConduiElement&& element, const FontInfo* font);
}
