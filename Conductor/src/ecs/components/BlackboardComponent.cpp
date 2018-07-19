#include <ecs/components/BlackboardComponent.h>

#include <ecs/ActorComponentVector.h>

bool ECS::Components::BlackboardComponent::TryCreateFromInfo(const BlackboardComponentInfo& componentInfo,
	const ActorComponentID reservedID, ActorComponentVector& destination)
{
	destination.Emplace<BlackboardComponent>(reservedID);
	return true;
}
