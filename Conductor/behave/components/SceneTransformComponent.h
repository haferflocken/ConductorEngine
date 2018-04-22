#pragma once

#include <behave/ActorComponent.h>
#include <math/Matrix4x4.h>

namespace Behave
{
class ActorComponentVector;

namespace Components
{
class SceneTransformComponentInfo;

/**
 * Actors with a SceneTransformComponent have a position, orientation, and scale within the scene.
 */
class SceneTransformComponent final : public ActorComponent
{
public:
	using Info = SceneTransformComponentInfo;

	static bool TryCreateFromInfo(const SceneTransformComponentInfo& componentInfo, const ActorComponentID reservedID,
		ActorComponentVector& destination);

	explicit SceneTransformComponent(const ActorComponentID id)
		: ActorComponent(id)
		, m_matrix()
	{}

	virtual ~SceneTransformComponent() {}

	// A 4x4 transform matrix in scene space.
	Math::Matrix4x4 m_matrix;
};
}
}
