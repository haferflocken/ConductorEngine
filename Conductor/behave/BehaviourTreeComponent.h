#pragma once

#include <asset/AssetHandle.h>
#include <behave/BehaviourForest.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <ecs/Component.h>

namespace Behave
{
/**
 * A BehaviourTreeComponent allows an Entity to run behaviour trees.
 */
class BehaviourTreeComponent final : public ECS::Component
{
public:
	static constexpr const char* k_typeName = "behaviour_tree_component";
	static const Util::StringHash k_typeHash;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	explicit BehaviourTreeComponent(const ECS::ComponentID id)
		: Component(id)
		, m_treeEvaluators()
	{}

	BehaviourTreeComponent(const BehaviourTreeComponent&) = delete;
	BehaviourTreeComponent& operator=(const BehaviourTreeComponent&) = delete;

	BehaviourTreeComponent(BehaviourTreeComponent&&) = default;
	BehaviourTreeComponent& operator=(BehaviourTreeComponent&&) = default;

	~BehaviourTreeComponent() {}

	Collection::Vector<Asset::AssetHandle<BehaviourForest>> m_referencedForests;
	Collection::Vector<BehaviourTreeEvaluator> m_treeEvaluators;
};
}
