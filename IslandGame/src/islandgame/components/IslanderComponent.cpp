#include <islandgame/components/IslanderComponent.h>

#include <mem/InspectorInfo.h>

namespace IslandGame::Components
{
const ECS::ComponentType IslanderComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfo IslanderComponent::k_inspectorInfo = MakeInspectorInfo(IslandGame::Components::IslanderComponent, 0);
}
