#include <behave/components/SceneTransformComponent.h>

#include <behave/ActorComponentVector.h>

bool Behave::Components::SceneTransformComponent::TryCreateFromInfo(const SceneTransformComponentInfo& componentInfo,
	const ActorComponentID reservedID, ActorComponentVector& destination)
{
	destination.Emplace<SceneTransformComponent>(reservedID);
	return true;
}
