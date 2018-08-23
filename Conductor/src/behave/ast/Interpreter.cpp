#include <behave/ast/Interpreter.h>

#include <dev/Dev.h>

namespace Behave::AST
{
Expression Interpreter::Compile(const Parse::Expression& parsedExpression) const
{
	// TODO(behave) compile the expression
	return Expression();
}

ExpressionResultType Interpreter::EvaluateExpression(const Expression& expression, const ECS::Entity& entity) const
{
	Dev::FatalAssert(expression.m_variant.IsAny(), "Cannot evaluate an invalid expression.");

	ExpressionResultType result;
	expression.m_variant.Match(
		[&](const BooleanLiteralExpression& boolVal)
		{
			result = ExpressionResultType::Make<bool>(boolVal.m_value);
		},
		[&](const NumericLiteralExpression& numVal)
		{
			result = ExpressionResultType::Make<double>(numVal.m_value);
		},
		[&](const ComponentTypeLiteralExpression& componentType)
		{
			result = ExpressionResultType::Make<ECS::ComponentType>(componentType.m_type);
		},
		[&](const TreeIdentifierExpression& treeIdentifier)
		{
			result = ExpressionResultType::Make<TreeIdentifier>(TreeIdentifier{ treeIdentifier.m_treeNameHash });
		},
		[&](const FunctionCallExpression& functionCall)
		{
			result = functionCall.m_boundFunction(*this, functionCall.m_arguments, entity);
		});

	return result;
}
}
