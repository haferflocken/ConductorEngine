#include <condui/components/StackingPanelComponent.h>

#include <mem/InspectorInfo.h>

namespace Condui
{
const ECS::ComponentType StackingPanelComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash StackingPanelComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Condui::StackingPanelComponent, 0);
}