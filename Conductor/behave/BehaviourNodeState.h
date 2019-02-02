#pragma once

#include <functional>

namespace Asset { template <typename TAsset> class AssetHandle; }
namespace Collection { template <typename T> class Vector; }

namespace ECS
{
class Entity;
class EntityManager;
}

namespace Behave
{
class BehaveContext;
class BehaviourForest;
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
	
	virtual EvaluateResult Evaluate(const BehaveContext& context,
		const Collection::Vector<Asset::AssetHandle<BehaviourForest>>& forests,
		ECS::Entity& entity,
		BehaviourTreeEvaluator& treeEvaluator,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions) = 0;

	virtual void NotifyChildFinished(const BehaviourNode* child, const EvaluateResult result) {}

	virtual void NotifyEarlyTermination(BehaviourTreeEvaluator& treeEvaluator) {}
};
}
