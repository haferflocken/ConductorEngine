#pragma once

#include <ecs/Component.h>

namespace Renderer
{
class CameraComponentInfo;

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
