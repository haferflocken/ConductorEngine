#include <ecs/systems/BehaviourTreeEvaluationSystem.h>

#include <behave/BehaviourTreeEvaluator.h>
#include <ecs/components/BehaviourTreeComponent.h>
#include <ecs/ECSGroup.h>
#include <ecs/EntityManager.h>

void ECS::Systems::BehaviourTreeEvaluationSystem::Update(
	EntityManager& entityManager,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void()>>& deferredFunctions) const
{
	// Update the entities in parallel, as their trees can't access other entities.
	// TODO make this parallel with a vector of deferred functions for each parallel list.
	std::for_each(ecsGroups.begin(), ecsGroups.end(),
		[&](const ECSGroupType& ecsGroup)
	{
		// Update this entity's tree evaluators.
		auto& entity = ecsGroup.Get<Entity>(entityManager);
		auto& behaviourTreeComponent = ecsGroup.Get<Components::BehaviourTreeComponent>(entityManager);
		for (auto& evaluator : behaviourTreeComponent.m_treeEvaluators)
		{
			evaluator.Update(entity, deferredFunctions, m_context);
		}

		// Destroy any evaluators which are no longer running a tree.
		const size_t removeIndex =
			behaviourTreeComponent.m_treeEvaluators.Partition([](const Behave::BehaviourTreeEvaluator& evaluator)
		{
			return evaluator.GetCurrentTree() != nullptr;
		});
		behaviourTreeComponent.m_treeEvaluators.Remove(removeIndex, behaviourTreeComponent.m_treeEvaluators.Size());
	});

	// Destroy any entities which are no longer running any trees.
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
