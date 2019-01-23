#pragma once

#include <mesh/StaticMesh.h>

#include <asset/AssetHandle.h>
#include <ecs/Component.h>

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
	static const Util::StringHash k_typeHash;

	static bool TryCreateFromFullSerialization(Asset::AssetManager& assetManager,
		const uint8_t*& bytes,
		const uint8_t* bytesEnd,
		const ECS::ComponentID reservedID,
		ECS::ComponentVector& destination);

	static void FullySerialize(const MeshComponent& component, Collection::Vector<uint8_t>& outBytes);

	static void ApplyFullSerialization(MeshComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd);

public:
	explicit MeshComponent(const ECS::ComponentID id)
		: Component(id)
		, m_meshHandle()
	{}

	MeshComponent(const MeshComponent&) = delete;
	MeshComponent& operator=(const MeshComponent&) = delete;

	MeshComponent(MeshComponent&&) = default;
	MeshComponent& operator=(MeshComponent&&) = default;

	~MeshComponent() {}

	Asset::AssetHandle<Mesh::StaticMesh> m_meshHandle;
};
}
