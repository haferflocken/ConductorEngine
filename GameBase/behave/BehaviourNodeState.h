#pragma once

#include <functional>

namespace Collection { template <typename T> class Vector; }

namespace Behave
{
class Actor;
class BehaviourNode;
class BehaviourTreeContext;
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
	
	virtual EvaluateResult Evaluate(Actor& actor, BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void()>>& deferredFunctions,
		const BehaviourTreeContext& context) = 0;

	virtual void NotifyChildFinished(const BehaviourNode* child, const EvaluateResult result) {}

	virtual void NotifyEarlyTermination(BehaviourTreeEvaluator& treeEvaluator) {}
};
}
