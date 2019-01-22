#pragma once

#include <asset/AssetManager.h>
#include <collection/Vector.h>

namespace Behave
{
class BehaviourForest;
class BehaviourTree;

class BehaviourTreeComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr const char* sk_typeName = "behaviour_tree_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ECS::ComponentInfo> LoadFromJSON(Asset::AssetManager& assetManager,
		const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }

	Collection::Vector<Asset::AssetHandle<BehaviourForest>> m_importedForests;
	Collection::Vector<Asset::AssetHandle<BehaviourForest>> m_behaviourForests;
	Collection::Vector<const BehaviourTree*> m_behaviourTrees;
};
}
