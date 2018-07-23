#pragma once

#include <behave/BehaveContext.h>
#include <ecs/components/BehaviourTreeComponent.h>
#include <ecs/components/BehaviourTreeComponentInfo.h>
#include <ecs/Entity.h>
#include <ecs/System.h>

#include <functional>

namespace Collection
{
template <typename T> class ArrayView;
template <typename T> class Vector;
}

namespace ECS
{
class Entity;
class EntityManager;

namespace Systems
{
/**
 * The behaviour tree evaluation system evaluates the behaviour trees of entities.
 */
class BehaviourTreeEvaluationSystem : public SystemTempl<
	Util::TypeList<>,
	Util::TypeList<Entity, Components::BehaviourTreeComponent>>
{
public:
	explicit BehaviourTreeEvaluationSystem(const Behave::BehaveContext& context)
		: m_context(context)
	{}

	void Update(EntityManager& entityManager,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void()>>& deferredFunctions) const;

private:
	Behave::BehaveContext m_context;
};
}
}
