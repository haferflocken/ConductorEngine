#include <renderer/MeshComponent.h>

#include <ecs/ComponentVector.h>

namespace Renderer
{
bool MeshComponent::TryCreateFromInfo(const MeshComponentInfo& componentInfo, const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	destination.Emplace<MeshComponent>(reservedID);
	return true;
}
}