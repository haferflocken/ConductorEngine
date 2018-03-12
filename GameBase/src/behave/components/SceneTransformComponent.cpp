#include <behave/components/SceneTransformComponent.h>

#include <behave/ActorComponentVector.h>

using namespace Behave;
using namespace Behave::Components;

bool SceneTransformComponent::TryCreateFromInfo(const SceneTransformComponentInfo& componentInfo,
	const ActorComponentID reservedID, ActorComponentVector& destination)
{
	destination.Emplace<SceneTransformComponent>(reservedID);
	return true;
}
