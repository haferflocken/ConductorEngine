#include <renderer/CameraComponent.h>

#include <ecs/ComponentVector.h>

namespace Renderer
{
const Util::StringHash CameraComponent::k_typeHash = Util::CalcHash(k_typeName);

bool CameraComponent::TryCreateFromInfo(
	Asset::AssetManager& assetManager,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	destination.Emplace<CameraComponent>(reservedID);
	return true;
}
}
