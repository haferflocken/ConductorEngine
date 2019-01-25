#include <scene/AnchorComponent.h>

namespace Scene
{
const ECS::ComponentType AnchorComponent::k_type{ Util::CalcHash(k_typeName) };
}
