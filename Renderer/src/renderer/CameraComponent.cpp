#include <renderer/CameraComponent.h>

namespace Renderer
{
const ECS::ComponentType CameraComponent::k_type{ Util::CalcHash(k_typeName) };
}
