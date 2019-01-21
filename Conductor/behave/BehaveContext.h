#pragma once

namespace ECS { class EntityManager; }

namespace Behave
{
namespace AST { class Interpreter; }

/**
 * A BehaveContext allows behaviour trees and behaviour systems to access data outside
 * of their entities/components when updating.
 * It should be used with care to avoid multithreaded memory sharing problems.
 */
class BehaveContext
{
public:
	const AST::Interpreter& m_interpreter;
	ECS::EntityManager& m_entityManager;
};
}
