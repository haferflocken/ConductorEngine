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
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::Normal;
	static constexpr const char* k_typeName = "behaviour_tree_component";
	static const ECS::ComponentType k_type;
	static const Mem::InspectorInfo k_inspectorInfo;

	static void FullySerialize(const BehaviourTreeComponent& component, Collection::Vector<uint8_t>& outBytes);

	static void ApplyFullSerialization(Asset::AssetManager& assetManager,
		BehaviourTreeComponent& component,
		const uint8_t*& bytes,
		const uint8_t* bytesEnd);

public:
	explicit BehaviourTreeComponent(const ECS::ComponentID id)
		: Component(id)
		, m_treeEvaluators()
	{}

	BehaviourTreeComponent(const BehaviourTreeComponent&) = delete;
	BehaviourTreeComponent& operator=(const BehaviourTreeComponent&) = delete;

	BehaviourTreeComponent(BehaviourTreeComponent&&) = default;
	BehaviourTreeComponent& operator=(BehaviourTreeComponent&&) = default;

	Collection::Vector<Asset::AssetHandle<BehaviourForest>> m_referencedForests;
	Collection::Vector<BehaviourTreeEvaluator> m_treeEvaluators;
};
}
