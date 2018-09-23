#include <behave/ast/Interpreter.h>

#include <behave/parse/BehaveParsedTree.h>

#include <dev/Dev.h>
#include <ecs/ComponentReflector.h>
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

Interpreter::Interpreter(const ECS::ComponentReflector& componentReflector)
	: m_componentReflector(componentReflector)
	, m_boundFunctionOverloads()
{
	BindFunction("Not", &Internal_Interpreter::Not);
	BindFunction("And", &Internal_Interpreter::And);
	BindFunction("Or", &Internal_Interpreter::Or);
	BindFunction("Xor", &Internal_Interpreter::Xor);
	BindFunction("Nor", &Internal_Interpreter::Nor);
	BindFunction("XNor", &Internal_Interpreter::XNor);

	BindFunction("+", &Internal_Interpreter::Add);
	BindFunction("-", &Internal_Interpreter::Subtract);
	BindFunction("*", &Internal_Interpreter::Multiply);
	BindFunction("/", &Internal_Interpreter::Divide);
	BindFunction("^", &Internal_Interpreter::Power);

	BindFunction("<", &Internal_Interpreter::LessThan);
	BindFunction("<=", &Internal_Interpreter::LessThanOrEqualTo);
	BindFunction(">", &Internal_Interpreter::GreaterThan);
	BindFunction(">=", &Internal_Interpreter::GreaterThanOrEqualTo);
	BindFunction("==", &Internal_Interpreter::EqualTo);
	BindFunction("!=", &Internal_Interpreter::NotEqualTo);

	BindFunction("Floor", &Internal_Interpreter::Floor);
	BindFunction("Ceil", &Internal_Interpreter::Ceil);
	BindFunction("Round", &Internal_Interpreter::Round);

	BindFunction("HasComponent", &Internal_Interpreter::HasComponent);
}

Interpreter::~Interpreter()
{}

ExpressionCompileResult Interpreter::Compile(Parse::Expression& parsedExpression) const
{
	ExpressionCompileResult result;

	parsedExpression.Match(
		[&](const Parse::NodeExpression& nodeExpression)
		{
			result = ExpressionCompileResult::Make<TypeCheckFailure>("A node expression cannot be interpreted.");
		},
		[&](Parse::FunctionCallExpression& functionCallExpression)
		{
			// Compile the function's argument expressions to get their types.
			// This is necessary so that the function name can be mangled for overload resolution.
			Collection::Vector<Expression> compiledArguments;
			Collection::Vector<ExpressionResultTypeString> compiledArgumentResultTypes;
			for (size_t i = 0, iEnd = functionCallExpression.m_arguments.Size(); i < iEnd; ++i)
			{
				ExpressionCompileResult argumentResult = Compile(functionCallExpression.m_arguments[i]);
				if (!argumentResult.Is<Expression>())
				{
					result = std::move(argumentResult);
					return;
				}

				Expression& compiledArgument = argumentResult.Get<Expression>();
				Dev::FatalAssert(compiledArgument.IsAny(),
					"No Behave AST compilation path may result in an invalid AST::Expression.");

				compiledArgumentResultTypes.Add(compiledArgument.GetResultType());
				compiledArguments.Add(std::move(compiledArgument));
			}

			// Validate that a function with the correct name has been bound.
			const auto* const overloadsEntry =
				m_boundFunctionOverloads.Find(functionCallExpression.m_functionNameHash);
			if (overloadsEntry == m_boundFunctionOverloads.end())
			{
				std::string message = "Encountered unknown function [";
				message += functionCallExpression.m_functionName;
				message += "].";

				result = ExpressionCompileResult::Make<TypeCheckFailure>(std::move(message));
				return;
			}

			// Try to find a perfect-match overload.
			const OverloadInfo* matchingOverload = nullptr;
			for (const auto& overloadInfo : overloadsEntry->second)
			{
				const ExpressionResultTypeString* const overloadArgumentTypes = overloadInfo.m_argumentTypeStrings;
				if (overloadInfo.m_numArguments != compiledArgumentResultTypes.Size())
				{
					continue;
				}
				bool isMatch = true;
				for (size_t i = 0, iEnd = overloadInfo.m_numArguments; i < iEnd; ++i)
				{
					if (overloadArgumentTypes[i] != compiledArgumentResultTypes[i])
					{
						isMatch = false;
						break;
					}
				}
				if (isMatch)
				{
					matchingOverload = &overloadInfo;
					break;
				}
			}

			// If there is not a perfect-match overload, try to find a compatible overload.
			// The only casting that can happen is from a specific component type to ECS::ComponentType.
			if (matchingOverload == nullptr)
			{
				for (const auto& overloadInfo : overloadsEntry->second)
				{
					const ExpressionResultTypeString* const overloadArgumentTypes = overloadInfo.m_argumentTypeStrings;
					if (overloadInfo.m_numArguments != compiledArgumentResultTypes.Size())
					{
						continue;
					}
					bool isMatch = true;
					for (size_t i = 0, iEnd = overloadInfo.m_numArguments; i < iEnd; ++i)
					{
						if (overloadArgumentTypes[i] != compiledArgumentResultTypes[i]
							&& overloadArgumentTypes[i] != ExpressionResultTypeString::Make<ECS::ComponentType>()
							&& !compiledArguments[i].Is<ComponentTypeLiteralExpression>())
						{
							isMatch = false;
							break;
						}
					}
					if (isMatch)
					{
						matchingOverload = &overloadInfo;
						break;
					}
				}
			}

			// Validate that an overload was found.
			if (matchingOverload == nullptr)
			{
				std::string message = "Function [";
				message += functionCallExpression.m_functionName;
				message += "] has no overload for types [";
				if (!compiledArgumentResultTypes.IsEmpty())
				{
					message += compiledArgumentResultTypes.Front().m_typeString;
					for (size_t i = 1, iEnd = compiledArgumentResultTypes.Size(); i < iEnd; ++i)
					{
						message += ", ";
						message += compiledArgumentResultTypes[i].m_typeString;
					}
				}
				message += "].";
				
				result = ExpressionCompileResult::Make<TypeCheckFailure>(std::move(message));
				return;
			}

			result = ExpressionCompileResult::Make<Expression>();
			result.Get<Expression>() = Expression::Make<FunctionCallExpression>(
				matchingOverload->m_boundFunction, std::move(compiledArguments));
		},
		[&](const Parse::IdentifierExpression& identifierExpression)
		{
			// TODO(behave) Verify the tree exists.
			result = ExpressionCompileResult::Make<Expression>();
			result.Get<Expression>() = Expression::Make<TreeIdentifier>(
				TreeIdentifier{ identifierExpression.m_treeNameHash });
		},
		[&](Parse::LiteralExpression& literalExpression)
		{
			Dev::FatalAssert(literalExpression.IsAny(), "Cannot compile an invalid Parse::Expression.");
			literalExpression.Match(
				[&](const Parse::NumericLiteral& numericLiteral)
				{
					result = ExpressionCompileResult::Make<Expression>();
					result.Get<Expression>() = Expression::Make<double>(numericLiteral.m_value);
				},
				[&](const Parse::StringLiteral& stringLiteral)
				{
					result = ExpressionCompileResult::Make<Expression>();
					result.Get<Expression>() = Expression::Make<std::string>(stringLiteral.m_value);
				},
				[&](const Parse::ResultLiteral&)
				{
					result = ExpressionCompileResult::Make<TypeCheckFailure>(
						"A result literal cannot be interpreted.");
				},
				[&](const Parse::BooleanLiteral& booleanLiteral)
				{
					result = ExpressionCompileResult::Make<Expression>();
					result.Get<Expression>() = Expression::Make<bool>(booleanLiteral.m_value);
				},
				[&](Parse::ComponentTypeLiteral& componentTypeLiteral)
				{
					// Verify the component type is registered.
					if (!m_componentReflector.IsRegistered(ECS::ComponentType(componentTypeLiteral.m_typeHash)))
					{
						std::string message = "Encountered unknown component type [";
						message += componentTypeLiteral.m_typeString;
						message += "].";

						result = ExpressionCompileResult::Make<TypeCheckFailure>(std::move(message));
						return;
					}

					result = ExpressionCompileResult::Make<Expression>();
					result.Get<Expression>() = Expression::Make<ComponentTypeLiteralExpression>(
						ECS::ComponentType(componentTypeLiteral.m_typeHash),
						std::move(componentTypeLiteral.m_typeString));
				});
		});

	return result;
}

ExpressionResult Interpreter::EvaluateExpression(const Expression& expression, ECS::EntityManager& entityManager,
	const ECS::Entity& entity) const
{
	Dev::FatalAssert(expression.IsAny(), "Cannot evaluate an invalid expression.");

	ExpressionResult result;
	expression.Match(
		[&](const None&)
		{
			result = ExpressionResult::Make<None>();
		},
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
		[&](const ComponentTypeLiteralExpression& componentTypeLiteral)
		{
			result = ExpressionResult::Make<ECS::ComponentType>(componentTypeLiteral.m_componentType);
		},
		[&](const TreeIdentifier& treeIdentifier)
		{
			result = ExpressionResult::Make<TreeIdentifier>(treeIdentifier);
		},
		[&](const FunctionCallExpression& functionCall)
		{
			result = functionCall.m_boundFunction(*this, functionCall.m_arguments, entityManager, entity);
		});

	return result;
}
}
