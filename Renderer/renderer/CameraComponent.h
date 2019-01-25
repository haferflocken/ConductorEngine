#pragma once

#include <ecs/Component.h>
#include <renderer/ViewIDs.h>

namespace Renderer
{
/**
 * The renderer views the world through entities with CameraComponents.
 * There should only be one camera per bgfx view active at any given time.
 */
class CameraComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::MemoryImaged;
	static constexpr const char* k_typeName = "camera_component";
	static const ECS::ComponentType k_type;

	explicit CameraComponent(const ECS::ComponentID id)
		: Component(id)
	{}

	virtual ~CameraComponent() {}

	uint16_t m_viewID{ k_sceneViewID };
	float m_nearDistance{ 0.02f };
	float m_farDistance{ 1000.f };
	float m_verticalFieldOfView{ 60.0f };
};
}
