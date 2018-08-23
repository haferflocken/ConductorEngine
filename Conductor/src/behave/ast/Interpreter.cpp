#include <behave/ast/Interpreter.h>

#include <behave/parse/BehaveParsedTree.h>

#include <dev/Dev.h>

namespace Behave::AST
{
ExpressionCompileResult Interpreter::Compile(const Parse::Expression& parsedExpression) const
{
	ExpressionCompileResult result;

	parsedExpression.m_variant.Match(
		[&](const Parse::NodeExpression& nodeExpression)
		{
			result = ExpressionCompileResult::Make<TypeCheckFailure>("A node expression cannot be interpreted.");
		},
		[&](const Parse::FunctionCallExpression& functionCallExpression)
		{
			const auto* const entry = m_boundFunctions.Find(functionCallExpression.m_functionNameHash);
			if (entry == m_boundFunctions.end())
			{
				std::string message = "Encountered unknown function [";
				message += functionCallExpression.m_functionName;
				message += "].";

				result = ExpressionCompileResult::Make<TypeCheckFailure>(std::move(message));
				return;
			}

			Collection::Vector<Expression> compiledArguments;
			for (const auto& argument : functionCallExpression.m_arguments)
			{
				ExpressionCompileResult argumentResult = Compile(argument);
				if (!argumentResult.Is<Expression>())
				{
					result = std::move(argumentResult);
					return;
				}
				// TODO(behave) type check the argument expressions
				compiledArguments.Add(std::move(argumentResult.Get<Expression>()));
			}

			result = ExpressionCompileResult::Make<Expression>();
			Expression& compiledFunctionCall = result.Get<Expression>();
			compiledFunctionCall.m_variant = decltype(Expression::m_variant)::Make<FunctionCallExpression>(
				entry->second, std::move(compiledArguments));
		},
		[&](const Parse::IdentifierExpression& identifierExpression)
		{
			// TODO(behave) Verify the tree exists.
			result = ExpressionCompileResult::Make<Expression>();
			Expression& compiledTreeIdentifier = result.Get<Expression>();
			compiledTreeIdentifier.m_variant = decltype(Expression::m_variant)::Make<TreeIdentifier>(
				TreeIdentifier{ identifierExpression.m_treeNameHash });
		},
		[&](const Parse::LiteralExpression& literalExpression)
		{
			Dev::FatalAssert(literalExpression.IsAny(), "Cannot compile an invalid Parse::Expression.");
			literalExpression.Match(
				[&](const Parse::NumericLiteral& numericLiteral)
				{
					result = ExpressionCompileResult::Make<Expression>();
					Expression& compiledExpression = result.Get<Expression>();
					compiledExpression.m_variant = decltype(Expression::m_variant)::Make<double>(
						numericLiteral.m_value);
				},
				[&](const Parse::StringLiteral& stringLiteral)
				{
					result = ExpressionCompileResult::Make<Expression>();
					Expression& compiledExpression = result.Get<Expression>();
					compiledExpression.m_variant = decltype(Expression::m_variant)::Make<std::string>(
						stringLiteral.m_value);
				},
				[&](const Parse::ResultLiteral&)
				{
					result = ExpressionCompileResult::Make<TypeCheckFailure>(
						"A result literal cannot be interpreted.");
				},
				[&](const Parse::BooleanLiteral& booleanLiteral)
				{
					result = ExpressionCompileResult::Make<Expression>();
					Expression& compiledExpression = result.Get<Expression>();
					compiledExpression.m_variant = decltype(Expression::m_variant)::Make<bool>(
						booleanLiteral.m_value);
				},
				[&](const Parse::ComponentTypeLiteral& componentTypeLiteral)
				{
					// TODO(behave) verify the component type exists
					result = ExpressionCompileResult::Make<Expression>();
					Expression& compiledExpression = result.Get<Expression>();
					compiledExpression.m_variant = decltype(Expression::m_variant)::Make<ECS::ComponentType>(
						componentTypeLiteral.m_typeHash);
				});
		});

	return result;
}

ExpressionResult Interpreter::EvaluateExpression(const Expression& expression, const ECS::Entity& entity) const
{
	Dev::FatalAssert(expression.m_variant.IsAny(), "Cannot evaluate an invalid expression.");

	ExpressionResult result;
	expression.m_variant.Match(
		[&](const bool& boolVal)
		{
			result = ExpressionResult::Make<bool>(boolVal);
		},
		[&](const double& numVal)
		{
			result = ExpressionResult::Make<double>(numVal);
		},
		[&](const std::string& strVal)
		{
			result = ExpressionResult::Make<std::string>(strVal);
		},
		[&](const ECS::ComponentType& componentType)
		{
			result = ExpressionResult::Make<ECS::ComponentType>(componentType);
		},
		[&](const TreeIdentifier& treeIdentifier)
		{
			result = ExpressionResult::Make<TreeIdentifier>(treeIdentifier);
		},
		[&](const FunctionCallExpression& functionCall)
		{
			result = functionCall.m_boundFunction(*this, functionCall.m_arguments, entity);
		});

	return result;
}
}
