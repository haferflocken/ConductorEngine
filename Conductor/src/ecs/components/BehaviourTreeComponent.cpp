#include <ecs/components/BehaviourTreeComponent.h>

#include <behave/BehaviourTree.h>
#include <ecs/ActorComponentVector.h>
#include <ecs/components/BehaviourTreeComponentInfo.h>

bool ECS::Components::BehaviourTreeComponent::TryCreateFromInfo(const BehaviourTreeComponentInfo& componentInfo,
	const ActorComponentID reservedID, ActorComponentVector& destination)
{
	BehaviourTreeComponent& component = destination.Emplace<BehaviourTreeComponent>(reservedID);

	for (const Behave::BehaviourTree* const behaviourTree : componentInfo.m_behaviourTrees)
	{
		Behave::BehaviourTreeEvaluator& treeEvaluator = component.m_treeEvaluators.Emplace();
		behaviourTree->GetRoot()->PushState(treeEvaluator);
	}

	return true;
}
