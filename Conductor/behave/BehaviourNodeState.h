#pragma once

#include <functional>

namespace Collection { template <typename T> class Vector; }

namespace ECS
{
class Entity;
class EntityManager;
}

namespace Behave
{
class BehaveContext;
class BehaviourNode;
class BehaviourTreeEvaluator;

enum class EvaluateResult
{
	Running,
	PushedNode,
	Success,
	Failure,
	Return
};

class BehaviourNodeState
{
public:
	virtual const BehaviourNode* GetNode() const = 0;
	
	virtual EvaluateResult Evaluate(ECS::Entity& entity, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions,
		const BehaveContext& context) = 0;

	virtual void NotifyChildFinished(const BehaviourNode* child, const EvaluateResult result) {}

	virtual void NotifyEarlyTermination(BehaviourTreeEvaluator& treeEvaluator) {}
};
}
