#pragma once

namespace Behave
{
class BehaviourTreeManager;

/**
 * A BehaveContext allows behaviour trees and behaviour systems to access data outside
 * of their actor/components when updating.
 * It should be used with care to avoid multithreaded memory sharing problems.
 */
class BehaveContext
{
public:
	explicit BehaveContext(const BehaviourTreeManager& behaviourTreeManager)
		: m_behaviourTreeManager(behaviourTreeManager)
	{}

	const BehaviourTreeManager& m_behaviourTreeManager;
};
}
