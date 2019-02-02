#include <behave/BehaviourTreeComponent.h>

#include <asset/AssetManager.h>
#include <behave/BehaviourNode.h>
#include <behave/BehaviourTree.h>
#include <ecs/ComponentVector.h>
#include <mem/DeserializeLittleEndian.h>
#include <mem/InspectorInfo.h>
#include <mem/SerializeLittleEndian.h>

namespace Behave
{
namespace Internal_BehaviourTreeComponent
{
bool DeserializeForestList(Asset::AssetManager& assetManager,
	Collection::Vector<Asset::AssetHandle<BehaviourForest>>& forests,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd)
{
	const auto maybeNumForests = Mem::LittleEndian::DeserializeUi32(bytes, bytesEnd);
	if (!maybeNumForests.second)
	{
		return false;
	}
	for (size_t i = 0; i < maybeNumForests.first; ++i)
	{
		Asset::CharType pathBuffer[Asset::k_maxPathLength];
		if (!Mem::LittleEndian::DeserializeString(bytes, bytesEnd, pathBuffer))
		{
			return false;
		}
		Asset::AssetHandle<BehaviourForest> handle = assetManager.RequestAsset<BehaviourForest>(
			File::MakePath(pathBuffer), Asset::LoadingMode::Immediate);
		forests.Add(std::move(handle));
	}
	return true;
}
}

const ECS::ComponentType BehaviourTreeComponent::k_type{ Util::CalcHash(k_typeName) };
const Mem::InspectorInfoTypeHash BehaviourTreeComponent::k_inspectorInfoTypeHash = MakeInspectorInfo(Behave::BehaviourTreeComponent, 0);

void BehaviourTreeComponent::FullySerialize(
	const BehaviourTreeComponent& component, Collection::Vector<uint8_t>& outBytes)
{
	// The state of the running trees isn't serialized.
	Mem::LittleEndian::Serialize(component.m_importedForests.Size(), outBytes);
	for (const auto& forest : component.m_importedForests)
	{
		const Asset::CharType* const forestPath = forest.GetAssetPath();
		Mem::LittleEndian::Serialize(forestPath, outBytes);
	}
	Mem::LittleEndian::Serialize(component.m_behaviourForests.Size(), outBytes);
	for (const auto& forest : component.m_behaviourForests)
	{
		const Asset::CharType* const forestPath = forest.GetAssetPath();
		Mem::LittleEndian::Serialize(forestPath, outBytes);
	}
	Mem::LittleEndian::Serialize(component.m_treeNameHashes.Size(), outBytes);
	for (const auto& nameHash : component.m_treeNameHashes)
	{
		const char* const treeName = Util::ReverseHash(nameHash);
		Mem::LittleEndian::Serialize(treeName, outBytes);
	}
}

void BehaviourTreeComponent::ApplyFullSerialization(Asset::AssetManager& assetManager,
	BehaviourTreeComponent& component,
	const uint8_t*& bytes,
	const uint8_t* bytesEnd)
{
	using namespace Internal_BehaviourTreeComponent;

	if (!DeserializeForestList(assetManager, component.m_importedForests, bytes, bytesEnd))
	{
		return;
	}
	if (!DeserializeForestList(assetManager, component.m_behaviourForests, bytes, bytesEnd))
	{
		return;
	}

	const auto maybeNumTreeNames = Mem::LittleEndian::DeserializeUi32(bytes, bytesEnd);
	if (!maybeNumTreeNames.second)
	{
		return;
	}
	for (size_t i = 0; i < maybeNumTreeNames.first; ++i)
	{
		char treeNameBuffer[64];
		if (!Mem::LittleEndian::DeserializeString(bytes, bytesEnd, treeNameBuffer))
		{
			return;
		}
		component.m_treeNameHashes.Add(Util::CalcHash(treeNameBuffer));
	}

	// Start the tree evaluators.
	component.m_referencedForests.AddAll(component.m_importedForests.GetConstView());
	component.m_referencedForests.AddAll(component.m_behaviourForests.GetConstView());
	for (const auto& treeNameHash : component.m_treeNameHashes)
	{
		const BehaviourTree* behaviourTree = nullptr;
		for (const auto& forestHandle : component.m_behaviourForests)
		{
			const BehaviourForest& forest = *forestHandle.TryGetAsset();
			behaviourTree = forest.FindTree(treeNameHash);
			if (behaviourTree != nullptr)
			{
				break;
			}
		}
		if (behaviourTree == nullptr)
		{
			AMP_LOG_WARNING("Failed to find behaviour tree \"%s\".", Util::ReverseHash(treeNameHash));
			continue;
		}

		Behave::BehaviourTreeEvaluator& treeEvaluator = component.m_treeEvaluators.Emplace();
		behaviourTree->GetRoot()->PushState(treeEvaluator);
	}
}
}
