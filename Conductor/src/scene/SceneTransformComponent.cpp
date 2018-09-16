#include <scene/SceneTransformComponent.h>

#include <ecs/ComponentVector.h>

bool Scene::SceneTransformComponent::TryCreateFromInfo(
	Asset::AssetManager& assetManager,
	const Scene::SceneTransformComponentInfo& componentInfo,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	destination.Emplace<SceneTransformComponent>(reservedID);
	return true;
}
