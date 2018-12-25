#pragma once

#include <collection/Pair.h>
#include <collection/Variant.h>
#include <collection/Vector.h>
#include <math/Matrix4x4.h>
#include <mem/UniquePtr.h>
#include <util/StringHash.h>

#include <string>

namespace ECS
{
class Entity;
class EntityInfoManager;
class EntityManager;
}

namespace Condui
{
struct ConduiElement;

/**
 * An element that displays a string.
 */
struct TextDisplayElement final
{
	std::string m_string{};
	float m_fontScale{ 1.0f };
};

/**
 * An element that can be typed in.
 */
struct TextInputElement final
{
	// The bounds of the text input rectangle as scales of the parent space.
	float m_xScale{ 1.0f };
	float m_yScale{ 1.0f };
	float m_fontScale{ 1.0f };
};

/**
 * An element that contains other elements transformed from itself.
 */
struct PanelElement final
{
	Collection::Vector<Math::Matrix4x4> m_childRelativeTransforms;
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
	PanelElement>
{
	using Variant::Variant;

	ConduiElement() = default;

	ConduiElement(Variant&& v)
		: Variant(std::move(v))
	{}

	template <typename T, typename... Args>
	static ConduiElement Make(Args&&... args)
	{
		return ConduiElement(Variant::Make<T, Args...>(std::forward<Args>(args)...));
	}
};

/**
 * The root of a tree of ConduiElements, providing them a root transform to orient from.
 */
struct ElementRoot final
{
	Math::Matrix4x4 m_uiTransform;
	ConduiElement m_element;
};

ConduiElement MakeTextDisplayElement(const char* const str, const float fontScale = 1.0f);
ConduiElement MakeTextInputElement(const float xScale, const float yScale, const float fontScale = 1.0f);
ConduiElement MakePanelElement(
	Collection::Vector<Collection::Pair<Math::Matrix4x4, ConduiElement>>&& childrenWithRelativeTransforms);

ECS::Entity& CreateConduiEntity(
	const ECS::EntityInfoManager& entityInfoManager,
	ECS::EntityManager& entityManager,
	const ConduiElement& element);

ECS::Entity& CreateConduiRootEntity(
	const ECS::EntityInfoManager& entityInfoManager,
	ECS::EntityManager& entityManager,
	const ElementRoot& elementRoot);
}
