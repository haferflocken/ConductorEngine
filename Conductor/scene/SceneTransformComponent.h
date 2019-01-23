#pragma once

#include <ecs/Component.h>
#include <math/Matrix4x4.h>

namespace Scene
{
/**
 * Entities with a SceneTransformComponent have a position, orientation, and scale within the scene (a transform).
 * If they have a parent entity with a SceneTransformComponent, their transform is relative to their parent's transform.
 */
class SceneTransformComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::MemoryImaged;
	static constexpr const char* k_typeName = "scene_transform_component";
	static const Util::StringHash k_typeHash;

	explicit SceneTransformComponent(const ECS::ComponentID id)
		: ECS::Component(id)
	{}

	~SceneTransformComponent() {}

	// A 4x4 transform matrix in scene space.
	Math::Matrix4x4 m_modelToWorldMatrix{};
	// A relative transform from the parent's transform.
	Math::Matrix4x4 m_childToParentMatrix{};
};
}
