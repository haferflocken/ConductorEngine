#pragma once

#include <file/Path.h>

namespace ECS
{
class ComponentReflector;
class EntityManager;
}

namespace Input { class CallbackRegistry; }
namespace Math { class Frustum; }
namespace Util { class StringHash; }

namespace Condui
{
struct ConduiElement;

// Registers all Condui component types.
void RegisterComponentTypes(ECS::ComponentReflector& componentReflector);

// Register all Condui ECS systems.
void RegisterSystems(ECS::EntityManager& entityManager,
	const Math::Frustum& sceneViewFrustum,
	Input::CallbackRegistry& callbackRegistry);
}
