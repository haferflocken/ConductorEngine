#include <mesh/MeshComponent.h>

#include <asset/AssetManager.h>
#include <ecs/ComponentVector.h>
#include <mem/DeserializeLittleEndian.h>
#include <mem/SerializeLittleEndian.h>

#include <mem/InspectorInfo.h>

namespace Mesh
{
const ECS::ComponentType MeshComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash MeshComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Mesh::MeshComponent, 1, m_meshHandle);

void Foo()
{

}

void MeshComponent::FullySerialize(const MeshComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	const Asset::CharType* const assetPath = component.m_meshHandle.GetAssetPath();
	if (assetPath != nullptr)
	{
		Mem::LittleEndian::Serialize(assetPath, outBytes);
	}
}

void MeshComponent::ApplyFullSerialization(
	Asset::AssetManager& assetManager, MeshComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	wchar_t assetPathBuffer[Asset::k_maxPathLength];
	if (Mem::LittleEndian::DeserializeString(bytes, bytesEnd, assetPathBuffer))
	{
		component.m_meshHandle = assetManager.RequestAsset<StaticMesh>(File::MakePath(assetPathBuffer));
	}
	else
	{
		component.m_meshHandle = Asset::AssetHandle<StaticMesh>();
	}
}
}
