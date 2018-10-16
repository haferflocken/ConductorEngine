#include <scene/AnchorComponent.h>

#include <ecs/ComponentVector.h>

namespace Scene
{
bool AnchorComponent::TryCreateFromInfo(Asset::AssetManager& assetManager, const AnchorComponentInfo& componentInfo,
	const ECS::ComponentID reservedID, ECS::ComponentVector& destination)
{
	destination.Emplace<AnchorComponent>(reservedID);
	return true;
}
}
