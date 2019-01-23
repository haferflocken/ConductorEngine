#include <mesh/MeshComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>

namespace Mesh
{
const Util::StringHash MeshComponent::k_typeHash = Util::CalcHash(k_typeName);

bool MeshComponent::TryCreateFromFullSerialization(Asset::AssetManager& assetManager,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	MeshComponent& meshComponent = destination.Emplace<MeshComponent>(reservedID);
	// TODO(info) meshComponent.m_meshHandle = assetManager.RequestAsset<StaticMesh>(componentInfo.m_meshFilePath);
	return true;
}

void MeshComponent::FullySerialize(const MeshComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	// TODO(info) serialize
}

void MeshComponent::ApplyFullSerialization(MeshComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	// TODO(info) apply
}
}