#pragma once

#include <behave/ActorID.h>

#include <Collection/PolyStack.h>
#include <Collection/Vector.h>

namespace Behave
{
class Actor;
class BehaviourCondition;
class BehaviourNodeState;
class BehaviourTree;
class BehaviourTreeContext;

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
	
	void Update(Actor& actor, Collection::Vector<std::function<void()>>& deferredFunctions,
		const BehaviourTreeContext& context);

private:
	Collection::PolyStack<BehaviourNodeState> m_callStack;
	Collection::Vector<DomainEntry> m_domainStack;
};
}
