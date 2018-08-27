#include <behave/BlackboardComponent.h>

#include <ecs/ComponentVector.h>

bool Behave::BlackboardComponent::TryCreateFromInfo(const BlackboardComponentInfo& componentInfo,
	const ECS::ComponentID reservedID, ECS::ComponentVector& destination)
{
	destination.Emplace<BlackboardComponent>(reservedID);
	return true;
}
