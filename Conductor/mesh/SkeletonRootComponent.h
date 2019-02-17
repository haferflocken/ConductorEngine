#pragma once

#include <collection/Vector.h>
#include <ecs/Component.h>

namespace Mesh
{
/**
 * An entity with a SkeletonRootComponent has the bones of its mesh (as defined by its MeshComponent) instantiated
 * as child entities and stores the necessary bone data here.
 */
class SkeletonRootComponent final : public ECS::Component
{
public:
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Normal;
	static constexpr const char* k_typeName = "skeleton_root_component";
	static const ECS::ComponentType k_type;
	static const Mem::InspectorInfoTypeHash k_inspectorInfoTypeHash;

	static void FullySerialize(const SkeletonRootComponent& component, Collection::Vector<uint8_t>& outBytes);

	static void ApplyFullSerialization(
		Asset::AssetManager& assetManager, SkeletonRootComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd);

public:
	explicit SkeletonRootComponent(const ECS::ComponentID id)
		: Component(id)
		, m_boneTransformComponentIDs()
	{}

	SkeletonRootComponent(const SkeletonRootComponent&) = delete;
	SkeletonRootComponent& operator=(const SkeletonRootComponent&) = delete;

	SkeletonRootComponent(SkeletonRootComponent&&) = default;
	SkeletonRootComponent& operator=(SkeletonRootComponent&&) = default;

	// The IDs of the SceneTransformComponents of the bone entities that make up this skeleton.
	Collection::Vector<ECS::ComponentID> m_boneTransformComponentIDs;
};
}
