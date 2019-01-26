#include <renderer/CameraComponent.h>

#include <mem/InspectorInfo.h>

namespace Renderer
{
const ECS::ComponentType CameraComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfo CameraComponent::k_inspectorInfo = MakeInspectorInfo(Renderer::CameraComponent, 0);
}
