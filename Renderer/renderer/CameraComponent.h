#pragma once

#include <ecs/Component.h>

namespace Renderer
{
class CameraComponentInfo;

/**
 * The renderer views the world through entities with CameraComponents.
 * There should only be one camera per bgfx view active at any given time.
 */
class CameraComponent final : public ECS::Component
{
public:
	using Info = CameraComponentInfo;

	static bool TryCreateFromInfo(const CameraComponentInfo& componentInfo, const ECS::ComponentID reservedID,
		ECS::ComponentVector& destination);

	explicit CameraComponent(const ECS::ComponentID id)
		: Component(id)
		, m_viewID(0)
		, m_verticalFieldOfView(60.0f)
	{}

	virtual ~CameraComponent() {}

	uint16_t m_viewID;
	float m_verticalFieldOfView;
};
}
