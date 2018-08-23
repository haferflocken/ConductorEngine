#include <behave/ast/Interpreter.h>

#include <behave/parse/BehaveParsedTree.h>

#include <dev/Dev.h>
#include <ecs/Entity.h>

namespace Behave::AST
{
namespace Internal_Interpreter
{
bool Not(const ECS::Entity&, bool val) { return !val; }
bool And(const ECS::Entity&, bool lhs, bool rhs) { return lhs && rhs; }
bool Or(const ECS::Entity&, bool lhs, bool rhs) { return lhs || rhs; }
bool Xor(const ECS::Entity&, bool lhs, bool rhs) { return lhs ^ rhs; }
bool Nor(const ECS::Entity&, bool lhs, bool rhs) { return !(lhs || rhs); }
bool XNor(const ECS::Entity&, bool lhs, bool rhs) { return lhs == rhs; }

double Add(const ECS::Entity&, double lhs, double rhs) { return lhs + rhs; }
double Subtract(const ECS::Entity&, double lhs, double rhs) { return lhs - rhs; }
double Multiply(const ECS::Entity&, double lhs, double rhs) { return lhs * rhs; }
double Divide(const ECS::Entity&, double lhs, double rhs) { return lhs / rhs; }
double Power(const ECS::Entity&, double lhs, double rhs) { return pow(lhs, rhs); }

bool LessThan(const ECS::Entity&, double lhs, double rhs) { return lhs < rhs; }
bool LessThanOrEqualTo(const ECS::Entity&, double lhs, double rhs) { return lhs <= rhs; }
bool GreaterThan(const ECS::Entity&, double lhs, double rhs) { return lhs > rhs; }
bool GreaterThanOrEqualTo(const ECS::Entity&, double lhs, double rhs) { return lhs >= rhs; }
bool EqualTo(const ECS::Entity&, double lhs, double rhs) { return lhs == rhs; }
bool NotEqualTo(const ECS::Entity&, double lhs, double rhs) { return lhs != rhs; }

double Floor(const ECS::Entity&, double val) { return floor(val); }
double Ceil(const ECS::Entity&, double val) { return ceil(val); }
double Round(const ECS::Entity&, double val) { return round(val); }

bool HasComponent(const ECS::Entity& entity, ECS::ComponentType componentType)
{
	return entity.FindComponentID(componentType) != ECS::ComponentID();
}
}

Interpreter::Interpreter()
	: m_boundFunctions()
{
	BindFunction(Util::CalcHash("Not"), &Internal_Interpreter::Not);
	BindFunction(Util::CalcHash("And"), &Internal_Interpreter::And);
	BindFunction(Util::CalcHash("Or"), &Internal_Interpreter::Or);
	BindFunction(Util::CalcHash("Xor"), &Internal_Interpreter::Xor);
	BindFunction(Util::CalcHash("Nor"), &Internal_Interpreter::Nor);
	BindFunction(Util::CalcHash("XNor"), &Internal_Interpreter::XNor);

	BindFunction(Util::CalcHash("+"), &Internal_Interpreter::Add);
	BindFunction(Util::CalcHash("-"), &Internal_Interpreter::Subtract);
	BindFunction(Util::CalcHash("*"), &Internal_Interpreter::Multiply);
	BindFunction(Util::CalcHash("/"), &Internal_Interpreter::Divide);
	BindFunction(Util::CalcHash("^"), &Internal_Interpreter::Power);

	BindFunction(Util::CalcHash("<"), &Internal_Interpreter::LessThan);
	BindFunction(Util::CalcHash("<="), &Internal_Interpreter::LessThanOrEqualTo);
	BindFunction(Util::CalcHash(">"), &Internal_Interpreter::GreaterThan);
	BindFunction(Util::CalcHash(">="), &Internal_Interpreter::GreaterThanOrEqualTo);
	BindFunction(Util::CalcHash("=="), &Internal_Interpreter::EqualTo);
	BindFunction(Util::CalcHash("!="), &Internal_Interpreter::NotEqualTo);

	BindFunction(Util::CalcHash("Floor"), &Internal_Interpreter::Floor);
	BindFunction(Util::CalcHash("Ceil"), &Internal_Interpreter::Ceil);
	BindFunction(Util::CalcHash("Round"), &Internal_Interpreter::Round);

	BindFunction(Util::CalcHash("HasComponent"), &Internal_Interpreter::HasComponent);
}

Interpreter::~Interpreter()
{}

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
