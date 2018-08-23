#pragma once

namespace Behave
{
namespace AST { class Interpreter; }
class BehaviourTreeManager;

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
		const AST::Interpreter& interpreter)
		: m_behaviourTreeManager(behaviourTreeManager)
		, m_interpreter(interpreter)
	{}

	const BehaviourTreeManager& m_behaviourTreeManager;
	const AST::Interpreter& m_interpreter;
};
}
