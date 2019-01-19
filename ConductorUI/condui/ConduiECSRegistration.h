#pragma once

#include <file/Path.h>

namespace ECS
{
class ComponentInfoFactory;
class ComponentReflector;
class EntityInfoManager;
class EntityManager;
}

namespace Input { class CallbackRegistry; }
namespace Math { class Frustum; }
namespace Util { class StringHash; }

namespace Condui
{
struct ConduiElement;

// Registers all Condui component types.
void RegisterComponentTypes(ECS::ComponentReflector& componentReflector,
	ECS::ComponentInfoFactory& componentInfoFactory);

// Register all Condui ECS systems.
void RegisterSystems(ECS::EntityManager& entityManager,
	const Math::Frustum& sceneViewFrustum,
	Input::CallbackRegistry& callbackRegistry);

// Registers all Condui EntityInfo from C++ rather than through JSON files.
// All Condui text is rendered in monospace fonts generated from monochrome bitmap files; the parameters here are for
// the default font.
void RegisterEntityInfo(ECS::EntityInfoManager& entityInfoManager,
	const uint16_t characterWidthPixels,
	const uint16_t characterHeightPixels,
	const File::Path& codePagePath);

// Gets the entity info name hash for the given element's type.
Util::StringHash GetEntityInfoNameHashFor(const ConduiElement& element);
}
