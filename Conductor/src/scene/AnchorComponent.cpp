#include <scene/AnchorComponent.h>

#include <mem/InspectorInfo.h>

namespace Scene
{
const ECS::ComponentType AnchorComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash AnchorComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Scene::AnchorComponent, 1, m_anchoringRadiusInChunks);
}
