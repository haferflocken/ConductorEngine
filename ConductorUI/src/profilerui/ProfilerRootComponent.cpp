#include <profilerui/ProfilerRootComponent.h>

#include <mem/InspectorInfo.h>

namespace ProfilerUI
{
const ECS::ComponentType ProfilerRootComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash ProfilerRootComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(ProfilerUI::ProfilerRootComponent, 0);
}
