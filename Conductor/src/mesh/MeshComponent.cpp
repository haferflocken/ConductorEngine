#include <mesh/MeshComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>
#include <mesh/MeshComponentInfo.h>

namespace Mesh
{
const Util::StringHash MeshComponent::k_typeHash = Util::CalcHash(k_typeName);

bool MeshComponent::TryCreateFromInfo(
	Asset::AssetManager& assetManager,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	MeshComponent& meshComponent = destination.Emplace<MeshComponent>(reservedID);
	// meshComponent.m_meshHandle = assetManager.RequestAsset<StaticMesh>(componentInfo.m_meshFilePath);
	return true;
}
}