#include <behave/systems/BehaviourTreeEvaluationSystem.h>

#include <behave/ActorComponentGroup.h>
#include <behave/ActorManager.h>
#include <behave/BehaviourTreeEvaluator.h>
#include <behave/components/BehaviourTreeComponent.h>

void Behave::Systems::BehaviourTreeEvaluationSystem::Update(ActorManager& actorManager, const BehaveContext& context,
	const Collection::ArrayView<ActorComponentGroupType>& actorComponentGroups) const
{
	// Update the actors in parallel, as their trees can't access other actors.
	// TODO When MSVC supports the <execution> header, make this parallel,
	//      with a vector of deferred functions for each parallel list.
	Collection::Vector<std::function<void()>> deferredFunctions;
	std::for_each(actorComponentGroups.begin(), actorComponentGroups.end(),
		[&](const ActorComponentGroupType& actorComponentGroup)
	{
		// Update this actor's tree evaluators.
		auto& actor = actorComponentGroup.Get<Actor>(actorManager);
		auto& behaviourTreeComponent = actorComponentGroup.Get<Components::BehaviourTreeComponent>(actorManager);
		for (auto& evaluator : behaviourTreeComponent.m_treeEvaluators)
		{
			evaluator.Update(actor, deferredFunctions, context);
		}

		// Destroy any evaluators which are no longer running a tree.
		const size_t removeIndex =
			behaviourTreeComponent.m_treeEvaluators.Partition([](const BehaviourTreeEvaluator& evaluator)
		{
			return evaluator.GetCurrentTree() != nullptr;
		});
		behaviourTreeComponent.m_treeEvaluators.Remove(removeIndex, behaviourTreeComponent.m_treeEvaluators.Size());
	});

	// Resolve deferred functions.
	for (auto& deferredFunction : deferredFunctions)
	{
		deferredFunction();
	}
	deferredFunctions.Clear();

	// Destroy any actors which are no longer running any trees.
	// TODO Is this desired?
	/*const size_t removeIndex = m_actors.Partition([](const Actor& actor)
	{
		return !actor.m_treeEvaluators.IsEmpty();
	});
	for (size_t i = removeIndex, iEnd = m_actors.Size(); i < iEnd; ++i)
	{
		const Actor& actor = m_actors[i];
		for (const auto& componentID : actor.m_components)
		{
			RemoveComponent(componentID);
		}
	}
	m_actors.Remove(removeIndex, m_actors.Size());*/
}
