#pragma once

#include <behave/BehaveContext.h>
#include <behave/BehaviourTreeComponent.h>
#include <behave/BehaviourTreeComponentInfo.h>
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
}

namespace Behave
{
/**
 * The behaviour tree evaluation system evaluates the behaviour trees of entities.
 */
class BehaviourTreeEvaluationSystem : public ECS::SystemTempl<
	Util::TypeList<>,
	Util::TypeList<ECS::Entity, Behave::BehaviourTreeComponent>>
{
public:
	explicit BehaviourTreeEvaluationSystem(const Behave::BehaveContext& context)
		: m_context(context)
	{}

	void Update(ECS::EntityManager& entityManager,
		const Collection::ArrayView<ECSGroupType>& ecsGroups,
		Collection::Vector<std::function<void()>>& deferredFunctions) const;

private:
	Behave::BehaveContext m_context;
};
}