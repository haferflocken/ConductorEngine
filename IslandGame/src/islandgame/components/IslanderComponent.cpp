#include <islandgame/components/IslanderComponent.h>

#include <behave/ActorComponentVector.h>

using namespace IslandGame;
using namespace IslandGame::Components;

bool IslanderComponent::TryCreateFromInfo(const IslanderComponentInfo& componentInfo,
	const Behave::ActorComponentID reservedID, Behave::ActorComponentVector& destination)
{
	destination.Emplace<IslanderComponent>(reservedID);
	return true;
}
