#pragma once

#include <ecs/Component.h>
#include <math/Matrix4x4.h>

namespace Scene
{
class SceneTransformComponentInfo;

/**
 * Entities with a SceneTransformComponent have a position, orientation, and scale within the scene (a transform).
 * If they have a parent entity with a SceneTransformComponent, their transform is relative to their parent's transform.
 */
class SceneTransformComponent final : public ECS::Component
{
public:
	using Info = SceneTransformComponentInfo;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const SceneTransformComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	explicit SceneTransformComponent(const ECS::ComponentID id)
		: ECS::Component(id)
	{}

	virtual ~SceneTransformComponent() {}

	// A 4x4 transform matrix in scene space.
	Math::Matrix4x4 m_modelToWorldMatrix{};
	// A relative transform from the parent's transform.
	Math::Matrix4x4 m_childToParentMatrix{};
};
}
