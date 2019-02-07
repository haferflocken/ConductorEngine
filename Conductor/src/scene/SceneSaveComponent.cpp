#include <scene/SceneSaveComponent.h>

#include <mem/InspectorInfo.h>

namespace Scene
{
const ECS::ComponentType SceneSaveComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash SceneSaveComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Scene::SceneSaveComponent, 0);
}
