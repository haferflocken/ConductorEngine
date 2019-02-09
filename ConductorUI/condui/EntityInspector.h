#pragma once

#include <condui/Condui.h>

namespace ECS
{
class ComponentReflector;
class Entity;
class EntityManager;
}

namespace Mem
{
struct InspectorInfo;
}

namespace Condui
{
ConduiElement MakeEntityInspector(
	const ECS::ComponentReflector& componentReflector,
	ECS::EntityManager& entityManager,
	ECS::Entity& subject,
	const float width,
	const float textHeight);
}
