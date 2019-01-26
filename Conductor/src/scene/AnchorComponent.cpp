#include <scene/AnchorComponent.h>

#include <mem/InspectorInfo.h>

namespace Scene
{
const ECS::ComponentType AnchorComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfo AnchorComponent::k_inspectorInfo = MakeInspectorInfo(Scene::AnchorComponent, 1, m_anchoringRadiusInChunks);
}
