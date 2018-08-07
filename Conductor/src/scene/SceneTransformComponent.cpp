#include <scene/SceneTransformComponent.h>

#include <ecs/ComponentVector.h>

bool Scene::SceneTransformComponent::TryCreateFromInfo(const Scene::SceneTransformComponentInfo& componentInfo,
	const ECS::ComponentID reservedID, ECS::ComponentVector& destination)
{
	destination.Emplace<SceneTransformComponent>(reservedID);
	return true;
}
