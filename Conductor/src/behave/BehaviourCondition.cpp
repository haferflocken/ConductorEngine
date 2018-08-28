#include <behave/BehaviourCondition.h>

#include <behave/ast/Interpreter.h>

namespace Behave
{
BehaviourCondition::BehaviourCondition(AST::Expression&& expression)
	: m_expression(std::move(expression))
{}

bool BehaviourCondition::Check(const AST::Interpreter& interpreter, ECS::EntityManager& entityManager,
	const ECS::Entity& entity) const
{
	// The expression was type checked before the condition was constructed, so it is safe to just get the bool.
	const AST::ExpressionResult result = interpreter.EvaluateExpression(m_expression, entityManager, entity);
	return result.Get<bool>();
}
}
