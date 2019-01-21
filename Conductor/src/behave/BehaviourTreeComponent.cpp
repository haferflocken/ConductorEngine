#include <behave/BehaviourTreeComponent.h>

#include <behave/BehaviourTreeComponentInfo.h>
#include <behave/BehaviourNode.h>
#include <behave/BehaviourTree.h>
#include <ecs/ComponentVector.h>

bool Behave::BehaviourTreeComponent::TryCreateFromInfo(
	Asset::AssetManager& assetManager,
	const BehaviourTreeComponentInfo& componentInfo,
	const ECS::ComponentID reservedID,
	ECS::ComponentVector& destination)
{
	BehaviourTreeComponent& component = destination.Emplace<BehaviourTreeComponent>(reservedID);

	component.m_referencedForests.AddAll(componentInfo.m_behaviourForests.GetConstView());
	component.m_referencedForests.AddAll(componentInfo.m_importedForests.GetConstView());

	for (const Behave::BehaviourTree* const behaviourTree : componentInfo.m_behaviourTrees)
	{
		Behave::BehaviourTreeEvaluator& treeEvaluator = component.m_treeEvaluators.Emplace();
		behaviourTree->GetRoot()->PushState(treeEvaluator);
	}

	return true;
}
