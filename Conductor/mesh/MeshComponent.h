#pragma once

#include <mesh/TriangleMesh.h>

#include <asset/AssetHandle.h>
#include <collection/Vector.h>
#include <ecs/Component.h>

#include <collection/Vector.h>

namespace Mesh
{
/**
 * Entities with a MeshComponent have a mesh drawn at their scene transform.
 */
class MeshComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Normal;
	static constexpr const char* k_typeName = "mesh_component";
	static const ECS::ComponentType k_type;
	static const Mem::InspectorInfoTypeHash k_inspectorInfoTypeHash;

	static void FullySerialize(const MeshComponent& component, Collection::Vector<uint8_t>& outBytes);

	static void ApplyFullSerialization(
		Asset::AssetManager& assetManager, MeshComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd);

public:
	explicit MeshComponent(const ECS::ComponentID id)
		: Component(id)
		, m_meshHandle()
		, m_boneToWorldMatrices()
	{}

	MeshComponent(const MeshComponent&) = delete;
	MeshComponent& operator=(const MeshComponent&) = delete;

	MeshComponent(MeshComponent&&) = default;
	MeshComponent& operator=(MeshComponent&&) = default;

	// The mesh in use by this component.
	Asset::AssetHandle<Mesh::TriangleMesh> m_meshHandle;
	// The world matricies of each bone in the mesh's skeleton.
	Collection::Vector<Math::Matrix4x4> m_boneToWorldMatrices;
};
}
