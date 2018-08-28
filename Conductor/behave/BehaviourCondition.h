#pragma once

#include <behave/ast/ASTTypes.h>

namespace ECS
{
class Entity;
class EntityManager;
}

namespace Behave
{
namespace AST { class Interpreter; }

class BehaviourCondition
{
public:
	BehaviourCondition(AST::Expression&& expression);

	bool Check(const AST::Interpreter& interpreter, ECS::EntityManager& entityManager,
		const ECS::Entity& entity) const;

private:
	AST::Expression m_expression;
};
}
