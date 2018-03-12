#include <behave/components/BlackboardComponent.h>

#include <behave/ActorComponentVector.h>

using namespace Behave;
using namespace Behave::Components;

bool BlackboardComponent::TryCreateFromInfo(const BlackboardComponentInfo& componentInfo,
	const ActorComponentID reservedID, ActorComponentVector& destination)
{
	destination.Emplace<BlackboardComponent>(reservedID);
	return true;
}
