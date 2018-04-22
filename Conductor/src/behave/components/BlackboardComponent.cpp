#include <behave/components/BlackboardComponent.h>

#include <behave/ActorComponentVector.h>

bool Behave::Components::BlackboardComponent::TryCreateFromInfo(const BlackboardComponentInfo& componentInfo,
	const ActorComponentID reservedID, ActorComponentVector& destination)
{
	destination.Emplace<BlackboardComponent>(reservedID);
	return true;
}
