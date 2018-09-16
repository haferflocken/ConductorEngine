#pragma once

#include <renderer/mesh/StaticMesh.h>

#include <asset/AssetHandle.h>
#include <ecs/Component.h>

#include <bgfx/bgfx.h>

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

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const MeshComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	explicit MeshComponent(const ECS::ComponentID id)
		: Component(id)
		, m_meshHandle()
		, m_vertexBuffer(BGFX_INVALID_HANDLE)
		, m_indexBuffer(BGFX_INVALID_HANDLE)
	{}

	MeshComponent(const MeshComponent&) = delete;
	MeshComponent& operator=(const MeshComponent&) = delete;

	MeshComponent(MeshComponent&&) = default;
	MeshComponent& operator=(MeshComponent&&) = default;

	virtual ~MeshComponent() {}

	Asset::AssetHandle<Mesh::StaticMesh> m_meshHandle;
	bgfx::VertexBufferHandle m_vertexBuffer;
	bgfx::IndexBufferHandle m_indexBuffer;
};
}
