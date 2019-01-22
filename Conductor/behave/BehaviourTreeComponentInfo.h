#pragma once

#include <asset/AssetManager.h>
#include <collection/Vector.h>

namespace JSON { class JSONObject; }

namespace Behave
{
class BehaviourForest;
class BehaviourTree;

class BehaviourTreeComponentInfo final
{
public:
	static constexpr const char* sk_typeName = "behaviour_tree_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<BehaviourTreeComponentInfo> LoadFromJSON(Asset::AssetManager& assetManager,
		const JSON::JSONObject& jsonObject);

	Collection::Vector<Asset::AssetHandle<BehaviourForest>> m_importedForests;
	Collection::Vector<Asset::AssetHandle<BehaviourForest>> m_behaviourForests;
	Collection::Vector<const BehaviourTree*> m_behaviourTrees;
};
}
