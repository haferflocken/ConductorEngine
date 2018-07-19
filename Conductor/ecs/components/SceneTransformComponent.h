#pragma once

#include <ecs/Component.h>
#include <math/Matrix4x4.h>

namespace ECS
{
class ComponentVector;

namespace Components
{
class SceneTransformComponentInfo;

/**
 * Entities with a SceneTransformComponent have a position, orientation, and scale within the scene.
 */
class SceneTransformComponent final : public Component
{
public:
	using Info = SceneTransformComponentInfo;

	static bool TryCreateFromInfo(const SceneTransformComponentInfo& componentInfo, const ComponentID reservedID,
		ComponentVector& destination);

	explicit SceneTransformComponent(const ComponentID id)
		: Component(id)
		, m_matrix()
	{}

	virtual ~SceneTransformComponent() {}

	// A 4x4 transform matrix in scene space.
	Math::Matrix4x4 m_matrix;
};
}
}
