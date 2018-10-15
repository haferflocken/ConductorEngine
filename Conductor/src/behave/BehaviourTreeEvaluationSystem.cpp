#include <behave/BehaviourTreeEvaluationSystem.h>

#include <behave/BehaviourTreeEvaluator.h>
#include <ecs/ECSGroup.h>

void Behave::BehaviourTreeEvaluationSystem::Update(
	const Unit::Time::Millisecond delta,
	const Collection::ArrayView<ECSGroupType>& ecsGroups,
	Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) const
{
	// Update the entities in parallel, as their trees can't access other entities.
	// TODO make this parallel with a vector of deferred functions for each parallel list.
	std::for_each(ecsGroups.begin(), ecsGroups.end(),
		[&](const ECSGroupType& ecsGroup)
	{
		// Update this entity's tree evaluators.
		auto& entity = ecsGroup.Get<ECS::Entity>();
		auto& behaviourTreeComponent = ecsGroup.Get<Behave::BehaviourTreeComponent>();
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
}
