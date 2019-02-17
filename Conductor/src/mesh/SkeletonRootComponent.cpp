#include <mesh/SkeletonRootComponent.h>

#include <mem/InspectorInfo.h>

namespace Mesh
{
const ECS::ComponentType SkeletonRootComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash SkeletonRootComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Mesh::SkeletonRootComponent, 0);

void SkeletonRootComponent::FullySerialize(
	const SkeletonRootComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	// TODO(mesh) serialize the SkeletonRootComponent
}

void SkeletonRootComponent::ApplyFullSerialization(
	Asset::AssetManager& assetManager, SkeletonRootComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	// TODO(mesh) deserialize the SkeletonRootComponent
}
}
