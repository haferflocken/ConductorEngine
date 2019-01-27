#include <renderer/CameraComponent.h>

#include <mem/InspectorInfo.h>

namespace Renderer
{
const ECS::ComponentType CameraComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash CameraComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Renderer::CameraComponent, 0);
}
