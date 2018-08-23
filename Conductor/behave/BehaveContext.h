#pragma once

namespace Behave
{
class BehaviourTreeManager;
namespace ConditionAST { class Interpreter; }

/**
 * A BehaveContext allows behaviour trees and behaviour systems to access data outside
 * of their entities/components when updating.
 * It should be used with care to avoid multithreaded memory sharing problems.
 */
class BehaveContext
{
public:
	explicit BehaveContext(
		const BehaviourTreeManager& behaviourTreeManager,
		const ConditionAST::Interpreter& interpreter)
		: m_behaviourTreeManager(behaviourTreeManager)
		, m_conditionASTInterpreter(interpreter)
	{}

	const BehaviourTreeManager& m_behaviourTreeManager;
	const ConditionAST::Interpreter& m_conditionASTInterpreter;
};
}
