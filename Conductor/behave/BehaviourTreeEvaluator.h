#pragma once

#include <collection/PolyStack.h>
#include <collection/Vector.h>
#include <ecs/EntityID.h>

namespace Asset
{
template <typename TAsset>
class AssetHandle;
}

namespace ECS
{
class Entity;
class EntityManager;
}

namespace Behave
{
class BehaveContext;
class BehaviourCondition;
class BehaviourForest;
class BehaviourNodeState;
class BehaviourTree;

/**
* A behaviour tree evaluator runs a behaviour tree.
*/
class BehaviourTreeEvaluator final
{
public:
	struct DomainEntry
	{
		DomainEntry() = default;

		DomainEntry(const BehaviourNodeState* state, const BehaviourCondition* condition)
			: m_state(state)
			, m_condition(condition)
		{}

		const BehaviourNodeState* m_state{ nullptr };
		const BehaviourCondition* m_condition{ nullptr };
	};

	BehaviourTreeEvaluator()
		: m_callStack()
		, m_domainStack()
	{}

	BehaviourTreeEvaluator(const BehaviourTreeEvaluator&) = delete;
	BehaviourTreeEvaluator& operator=(const BehaviourTreeEvaluator&) = delete;

	BehaviourTreeEvaluator(BehaviourTreeEvaluator&&) = default;
	BehaviourTreeEvaluator& operator=(BehaviourTreeEvaluator&&) = default;

	~BehaviourTreeEvaluator() {}

	const BehaviourTree* GetCurrentTree() const;

	Collection::PolyStack<BehaviourNodeState>& GetCallStack() { return m_callStack; }
	Collection::Vector<DomainEntry>& GetDomainStack() { return m_domainStack; }

	void Update(const BehaveContext& context,
		const Collection::Vector<Asset::AssetHandle<BehaviourForest>>& forests,
		ECS::Entity& entity,
		Collection::Vector<std::function<void(ECS::EntityManager&)>>& deferredFunctions);

private:
	Collection::PolyStack<BehaviourNodeState> m_callStack;
	Collection::Vector<DomainEntry> m_domainStack;
};
}
