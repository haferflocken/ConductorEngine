#pragma once

#include <ecs/Component.h>
#include <renderer/ViewIDs.h>

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

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const CameraComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	explicit CameraComponent(const ECS::ComponentID id)
		: Component(id)
	{}

	virtual ~CameraComponent() {}

	uint16_t m_viewID{ k_sceneViewID };
	float m_nearDistance{ 0.1f };
	float m_farDistance{ 1000.f };
	float m_verticalFieldOfView{ 60.0f };
};
}
