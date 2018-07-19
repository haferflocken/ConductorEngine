#include <islandgame/components/IslanderComponent.h>

#include <ecs/ComponentVector.h>

bool IslandGame::Components::IslanderComponent::TryCreateFromInfo(const IslanderComponentInfo& componentInfo,
	const ECS::ComponentID reservedID, ECS::ComponentVector& destination)
{
	destination.Emplace<IslanderComponent>(reservedID);
	return true;
}
