#pragma once

namespace Behave
{
class BehaviourTreeManager;

/**
 * A BehaviourTreeContext allows behaviour trees to access data outside of their actor/components when updating.
 * It should never allow read access to other actors within the same actor manager.
 */
class BehaviourTreeContext
{
public:
	explicit BehaviourTreeContext(const BehaviourTreeManager& behaviourTreeManager)
		: m_behaviourTreeManager(behaviourTreeManager)
	{}

	const BehaviourTreeManager& m_behaviourTreeManager;
};
}
