#include <islandgame/components/IslanderComponent.h>

#include <ecs/ActorComponentVector.h>

bool IslandGame::Components::IslanderComponent::TryCreateFromInfo(const IslanderComponentInfo& componentInfo,
	const ECS::ActorComponentID reservedID, ECS::ActorComponentVector& destination)
{
	destination.Emplace<IslanderComponent>(reservedID);
	return true;
}
