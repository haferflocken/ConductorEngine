#include <scene/SceneSaveComponent.h>

#include <mem/InspectorInfo.h>

namespace Scene
{
const ECS::ComponentType SceneSaveComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfo SceneSaveComponent::k_inspectorInfo = MakeInspectorInfo(Scene::SceneSaveComponent, 0);
}
