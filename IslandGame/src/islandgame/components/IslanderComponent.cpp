#include <islandgame/components/IslanderComponent.h>

#include <ecs/ComponentVector.h>

const Util::StringHash IslandGame::Components::IslanderComponent::k_typeHash = Util::CalcHash(k_typeName);

bool IslandGame::Components::IslanderComponent::TryCreateFromInfo(
	Asset::AssetManager& assetManager,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	destination.Emplace<IslanderComponent>(reservedID);
	return true;
}
