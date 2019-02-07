#include <islandgame/components/IslanderComponent.h>

#include <mem/InspectorInfo.h>

namespace IslandGame::Components
{
const ECS::ComponentType IslanderComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash IslanderComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(IslandGame::Components::IslanderComponent, 0);
}
