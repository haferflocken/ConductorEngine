#include <behave/BehaviourCondition.h>

#include <behave/conditionast/Interpreter.h>

namespace Behave
{
BehaviourCondition::BehaviourCondition(ConditionAST::Expression&& expression)
	: m_expression(std::move(expression))
{}

bool BehaviourCondition::Check(const ConditionAST::Interpreter& interpreter, const ECS::Entity& entity) const
{
	// The expression was type checked before the condition was constructed, so it is safe to just get the bool.
	const ConditionAST::ExpressionResultType result = interpreter.EvaluateExpression(m_expression, entity);
	return result.Get<bool>();
}
}
