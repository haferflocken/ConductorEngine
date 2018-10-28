#include <scene/AnchorComponent.h>

#include <ecs/ComponentVector.h>
#include <scene/AnchorComponentInfo.h>

namespace Scene
{
bool AnchorComponent::TryCreateFromInfo(Asset::AssetManager& assetManager, const AnchorComponentInfo& componentInfo,
	const ECS::ComponentID reservedID, ECS::ComponentVector& destination)
{
	destination.Emplace<AnchorComponent>(reservedID, componentInfo.m_anchoringRadiusInChunks);
	return true;
}
}