#pragma once

#include <mesh/StaticMesh.h>

#include <asset/AssetHandle.h>
#include <ecs/Component.h>

namespace Mesh
{
class MeshComponentInfo;

/**
 * Entities with a MeshComponent have a mesh drawn at their scene transform.
 */
class MeshComponent final : public ECS::Component
{
public:
	using Info = MeshComponentInfo;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const MeshComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	explicit MeshComponent(const ECS::ComponentID id)
		: Component(id)
		, m_meshHandle()
	{}

	MeshComponent(const MeshComponent&) = delete;
	MeshComponent& operator=(const MeshComponent&) = delete;

	MeshComponent(MeshComponent&&) = default;
	MeshComponent& operator=(MeshComponent&&) = default;

	virtual ~MeshComponent() {}

	Asset::AssetHandle<Mesh::StaticMesh> m_meshHandle;
};
}
