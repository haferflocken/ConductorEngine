#include <mesh/SkeletonRootComponent.h>

#include <mem/DeserializeLittleEndian.h>
#include <mem/InspectorInfo.h>
#include <mem/SerializeLittleEndian.h>
#include <scene/SceneTransformComponent.h>

namespace Mesh
{
const ECS::ComponentType SkeletonRootComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash SkeletonRootComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Mesh::SkeletonRootComponent, 0);

void SkeletonRootComponent::FullySerialize(
	const SkeletonRootComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	const uint16_t numBones = static_cast<uint16_t>(component.m_boneTransformComponentIDs.Size());
	Mem::LittleEndian::Serialize(numBones, outBytes);

	for (const auto& componentID : component.m_boneTransformComponentIDs)
	{
		Mem::LittleEndian::Serialize(uint64_t(componentID.GetUniqueID()), outBytes);
	}
}

void SkeletonRootComponent::ApplyFullSerialization(
	Asset::AssetManager& assetManager, SkeletonRootComponent& component, const uint8_t*& bytes, const uint8_t* bytesEnd)
{
	const auto maybeNumBones = Mem::LittleEndian::DeserializeUi16(bytes, bytesEnd);
	if (!maybeNumBones.second)
	{
		return;
	}
	const uint32_t numBones = maybeNumBones.first;
	component.m_boneTransformComponentIDs.Resize(numBones);
	for (size_t i = 0; i < numBones; ++i)
	{
		const auto maybeComponentUniqueID = Mem::LittleEndian::DeserializeUi64(bytes, bytesEnd);
		if (!maybeComponentUniqueID.second)
		{
			return;
		}

		const uint64_t componentUniqueID = maybeComponentUniqueID.first;
		component.m_boneTransformComponentIDs[i] =
			ECS::ComponentID(Scene::SceneTransformComponent::k_type, componentUniqueID);
	}
}
}
