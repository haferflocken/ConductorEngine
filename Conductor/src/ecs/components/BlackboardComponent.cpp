#include <ecs/components/BlackboardComponent.h>

#include <ecs/ComponentVector.h>

bool ECS::Components::BlackboardComponent::TryCreateFromInfo(const BlackboardComponentInfo& componentInfo,
	const ComponentID reservedID, ComponentVector& destination)
{
	destination.Emplace<BlackboardComponent>(reservedID);
	return true;
}
