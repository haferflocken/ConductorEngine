#include <ecs/components/SceneTransformComponent.h>

#include <ecs/ActorComponentVector.h>

bool ECS::Components::SceneTransformComponent::TryCreateFromInfo(const SceneTransformComponentInfo& componentInfo,
	const ActorComponentID reservedID, ActorComponentVector& destination)
{
	destination.Emplace<SceneTransformComponent>(reservedID);
	return true;
}
