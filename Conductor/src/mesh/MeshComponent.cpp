#include <mesh/MeshComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>

namespace Mesh
{
bool MeshComponent::TryCreateFromInfo(
	Asset::AssetManager& assetManager,
	const MeshComponentInfo& componentInfo,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	MeshComponent& meshComponent = destination.Emplace<MeshComponent>(reservedID);
	meshComponent.m_meshHandle = assetManager.RequestAsset<StaticMesh>("TODO(renderer)");
	return true;
}
}