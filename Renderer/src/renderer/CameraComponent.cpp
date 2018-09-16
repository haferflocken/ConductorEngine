#include <renderer/CameraComponent.h>

#include <ecs/ComponentVector.h>

namespace Renderer
{
bool CameraComponent::TryCreateFromInfo(
	Asset::AssetManager& assetManager,
	const CameraComponentInfo& componentInfo,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	destination.Emplace<CameraComponent>(reservedID);
	return true;
}
}
