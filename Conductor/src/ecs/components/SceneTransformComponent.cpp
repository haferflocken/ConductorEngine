#include <ecs/components/SceneTransformComponent.h>

#include <ecs/ComponentVector.h>

bool ECS::Components::SceneTransformComponent::TryCreateFromInfo(const SceneTransformComponentInfo& componentInfo,
	const ComponentID reservedID, ComponentVector& destination)
{
	destination.Emplace<SceneTransformComponent>(reservedID);
	return true;
}
