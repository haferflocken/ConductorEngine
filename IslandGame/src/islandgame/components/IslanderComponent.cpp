#include <islandgame/components/IslanderComponent.h>

#include <behave/ActorComponentVector.h>

bool IslandGame::Components::IslanderComponent::TryCreateFromInfo(const IslanderComponentInfo& componentInfo,
	const Behave::ActorComponentID reservedID, Behave::ActorComponentVector& destination)
{
	destination.Emplace<IslanderComponent>(reservedID);
	return true;
}
