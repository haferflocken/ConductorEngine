#pragma once

namespace ECS
{
class ComponentInfoFactory;
class ComponentReflector;
class EntityInfoManager;
class EntityManager;
}

namespace Util { class StringHash; }

namespace Condui
{
struct ConduiElement;

// Registers all Condui component types.
void RegisterComponentTypes(ECS::ComponentReflector& componentReflector,
	ECS::ComponentInfoFactory& componentInfoFactory);

// Register all Condui ECS systems.
void RegisterSystems(ECS::EntityManager& entityManager);

// Registers all Condui EntityInfo from C++ rather than through JSON files.
void RegisterEntityInfo(ECS::EntityInfoManager& entityInfoManager);

// Gets the entity info name hash for the given element's type.
Util::StringHash GetEntityInfoNameHashFor(const ConduiElement& element);
}
