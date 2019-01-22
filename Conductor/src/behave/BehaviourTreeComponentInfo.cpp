#include <behave/BehaviourTreeComponentInfo.h>

#include <behave/BehaviourForest.h>
#include <json/JSONTypes.h>
#include <mem/UniquePtr.h>
#include <util/StringHash.h>

namespace Internal_BehaviourTreeComponentInfo
{
const Util::StringHash k_behaviourTreeFilesHash = Util::CalcHash("behaviour_tree_files");
const Util::StringHash k_behaviourTreesHash = Util::CalcHash("behaviour_trees");
}

const Util::StringHash Behave::BehaviourTreeComponentInfo::sk_typeHash =
	Util::CalcHash(BehaviourTreeComponentInfo::sk_typeName);

Mem::UniquePtr<Behave::BehaviourTreeComponentInfo> Behave::BehaviourTreeComponentInfo::LoadFromJSON(
	Asset::AssetManager& assetManager,
	const JSON::JSONObject& jsonObject)
{
	using namespace Internal_BehaviourTreeComponentInfo;

	auto componentInfo = Mem::MakeUnique<BehaviourTreeComponentInfo>();

	// Load the specified .behave files.
	const JSON::JSONArray* const behaviourTreeFilesArray = jsonObject.FindArray(k_behaviourTreeFilesHash);
	if (behaviourTreeFilesArray == nullptr)
	{
		AMP_LOG_WARNING("Failed to find behaviour tree file array for behaviour_tree_component.");
		return nullptr;
	}

	for (const auto& value : *behaviourTreeFilesArray)
	{
		if (value->GetType() != JSON::ValueType::String)
		{
			AMP_LOG_WARNING("Encountered a non-string element in an entity info behaviour tree file array.");
			continue;
		}
		const JSON::JSONString& valueString = static_cast<const JSON::JSONString&>(*value);
		const File::Path forestPath = File::MakePath(valueString.m_string.c_str());
		Asset::AssetHandle<BehaviourForest> forestHandle =
			assetManager.RequestAsset<BehaviourForest>(forestPath, Asset::LoadingMode::Immediate);
		if (forestHandle.TryGetAsset() == nullptr)
		{
			AMP_LOG_WARNING("Failed to load behaviour forest \"%s\".", valueString.m_string.c_str());
			continue;
		}
		componentInfo->m_behaviourForests.Add(std::move(forestHandle));
	}

	// Find the required BehaviourTrees in the loaded forests.
	const JSON::JSONArray* const behaviourTreesArray = jsonObject.FindArray(k_behaviourTreesHash);
	if (behaviourTreesArray == nullptr)
	{
		AMP_LOG_WARNING("Failed to find behaviour tree array for behaviour_tree_component.");
		return nullptr;
	}

	for (const auto& value : *behaviourTreesArray)
	{
		if (value->GetType() != JSON::ValueType::String)
		{
			AMP_LOG_WARNING("Encountered a non-string element in an entity info behaviour tree array.");
			continue;
		}
		const JSON::JSONString& valueString = static_cast<const JSON::JSONString&>(*value);

		const BehaviourTree* behaviourTree = nullptr;
		for (const auto& forestHandle : componentInfo->m_behaviourForests)
		{
			const BehaviourForest& forest = *forestHandle.TryGetAsset();
			behaviourTree = forest.FindTree(valueString.m_hash);
			if (behaviourTree != nullptr)
			{
				break;
			}
		}
		if (behaviourTree == nullptr)
		{
			AMP_LOG_WARNING("Failed to find behaviour tree \"%s\".", valueString.m_string.c_str());
			continue;
		}
		componentInfo->m_behaviourTrees.Add(behaviourTree);
	}

	// Request the imported forests, all forests they import, and so on (recursively).
	Collection::Vector<const Collection::Vector<std::string>*> pendingImports;
	for (const auto& forestHandle : componentInfo->m_behaviourForests)
	{
		const BehaviourForest& forest = *forestHandle.TryGetAsset();
		pendingImports.Add(&forest.GetImports());
	}

	while (!pendingImports.IsEmpty())
	{
		const auto* const toImport = pendingImports.Back();
		pendingImports.RemoveLast();

		for (const auto& importPathString : *toImport)
		{
			const File::Path importPath = File::MakePath(importPathString.c_str());

			Asset::AssetHandle<BehaviourForest> forestHandle =
				assetManager.RequestAsset<BehaviourForest>(importPath, Asset::LoadingMode::Immediate);

			const BehaviourForest* const forest = forestHandle.TryGetAsset();
			if (forest == nullptr)
			{
				AMP_LOG_WARNING("Failed to load imported behaviour forest \"%s\".", importPathString.c_str());
				continue;
			}

			if (componentInfo->m_importedForests.IndexOf(forestHandle)
				== componentInfo->m_importedForests.sk_InvalidIndex)
			{
				componentInfo->m_importedForests.Add(std::move(forestHandle));
				pendingImports.Add(&forest->GetImports());
			}
		}
	}

	return componentInfo;
}
