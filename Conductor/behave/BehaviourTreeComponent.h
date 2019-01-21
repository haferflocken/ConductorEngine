#pragma once

#include <asset/AssetHandle.h>
#include <behave/BehaviourForest.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <ecs/Component.h>

namespace Behave
{
class BehaviourTreeComponentInfo;

/**
 * A BehaviourTreeComponent allows an Entity to run behaviour trees.
 */
class BehaviourTreeComponent final : public ECS::Component
{
public:
	using Info = BehaviourTreeComponentInfo;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const BehaviourTreeComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	explicit BehaviourTreeComponent(const ECS::ComponentID id)
		: Component(id)
		, m_treeEvaluators()
	{}

	BehaviourTreeComponent(const BehaviourTreeComponent&) = delete;
	BehaviourTreeComponent& operator=(const BehaviourTreeComponent&) = delete;

	BehaviourTreeComponent(BehaviourTreeComponent&&) = default;
	BehaviourTreeComponent& operator=(BehaviourTreeComponent&&) = default;

	virtual ~BehaviourTreeComponent() {}

	Collection::Vector<Asset::AssetHandle<BehaviourForest>> m_referencedForests;
	Collection::Vector<BehaviourTreeEvaluator> m_treeEvaluators;
};
}
