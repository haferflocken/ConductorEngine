#pragma once

#include <ecs/Component.h>

namespace Renderer
{
class MeshComponentInfo;

/**
 * Entities with a MeshComponent have a mesh drawn at their scene transform.
 */
class MeshComponent final : public ECS::Component
{
public:
	using Info = MeshComponentInfo;

	static bool TryCreateFromInfo(const MeshComponentInfo& componentInfo, const ECS::ComponentID reservedID,
		ECS::ComponentVector& destination);

	explicit MeshComponent(const ECS::ComponentID id)
		: Component(id)
	{}

	MeshComponent(const MeshComponent&) = delete;
	MeshComponent& operator=(const MeshComponent&) = delete;

	MeshComponent(MeshComponent&&) = default;
	MeshComponent& operator=(MeshComponent&&) = default;

	virtual ~MeshComponent() {}
};
}
