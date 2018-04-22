#include <behave/components/BehaviourTreeComponent.h>

#include <behave/ActorComponentVector.h>
#include <behave/BehaviourNode.h>
#include <behave/BehaviourTree.h>
#include <behave/components/BehaviourTreeComponentInfo.h>

bool Behave::Components::BehaviourTreeComponent::TryCreateFromInfo(const BehaviourTreeComponentInfo& componentInfo,
	const ActorComponentID reservedID, ActorComponentVector& destination)
{
	BehaviourTreeComponent& component = destination.Emplace<BehaviourTreeComponent>(reservedID);

	for (const BehaviourTree* const behaviourTree : componentInfo.m_behaviourTrees)
	{
		BehaviourTreeEvaluator& treeEvaluator = component.m_treeEvaluators.Emplace();
		behaviourTree->GetRoot()->PushState(treeEvaluator);
	}

	return true;
}
