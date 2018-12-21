#pragma once

namespace ECS { class EntityInfoManager; }
namespace Util { class StringHash; }

namespace Condui
{
struct ConduiElement;

// Registers all Condui EntityInfo from C++ rather than through JSON files.
void RegisterEntityInfo(ECS::EntityInfoManager& entityInfoManager);

// Gets the entity info name hash for the given element's type.
Util::StringHash GetEntityInfoNameHashFor(const ConduiElement& element);
}
