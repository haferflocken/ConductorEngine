#pragma once

#include <collection/Pair.h>
#include <collection/Variant.h>
#include <collection/Vector.h>
#include <math/Matrix4x4.h>

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
 * An element that contains other elements transformed from itself.
 */
struct PanelElement final
{
	Collection::Vector<Math::Matrix4x4> m_childRelativeTransforms;
	Collection::Vector<ConduiElement> m_children;
};

/**
 * A union of all element types.
 */
struct ConduiElement final : public Collection::Variant<
	TextDisplayElement,
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
