#pragma once

#include <behave/ast/ASTTypes.h>
#include <behave/ast/BoundFunction.h>
#include <behave/ast/ExpressionResultType.h>

#include <collection/Variant.h>
#include <collection/VectorMap.h>
#include <util/StringHash.h>

namespace Behave::Parse { struct Expression; }
namespace ECS { class Entity; }

namespace Behave::AST
{
struct TypeCheckFailure
{
	explicit TypeCheckFailure(const char* message)
		: m_message(message)
	{}

	explicit TypeCheckFailure(std::string&& message)
		: m_message(std::move(message))
	{}

	std::string m_message;
};

using ExpressionCompileResult = Collection::Variant<Expression, TypeCheckFailure>;

/**
 * An interpreter that evaluates AST expressions.
 */
class Interpreter
{
public:
	// Compile a parsed expression into an executable expression.
	ExpressionCompileResult Compile(const Parse::Expression& parsedExpression) const;

	// Evaluate an AST::Expression on the given entity.
	ExpressionResult EvaluateExpression(const Expression& expression, const ECS::Entity& entity) const;

	// Binds a function so that it can be called with AST::Expressions as arguments.
	template <typename ReturnType, typename... ArgumentTypes>
	void BindFunction(const Util::StringHash functionNameHash,
		ReturnType(*func)(const ECS::Entity&, ArgumentTypes...));

private:
	Collection::VectorMap<Util::StringHash, BoundFunction> m_boundFunctions;
};
}

namespace Behave::AST
{
template <typename ReturnType, typename... ArgumentTypes>
inline void Interpreter::BindFunction(const Util::StringHash functionNameHash,
	ReturnType(*func)(const ECS::Entity&, ArgumentTypes...))
{
	constexpr ExpressionResultTypes k_returnType =
		std::is_same_v<ReturnType, bool> ? ExpressionResultTypes::Boolean
		: std::is_same_v<ReturnType, double> ? ExpressionResultTypes::Number
		: std::is_same_v<ReturnType, std::string> ? ExpressionResultTypes::String
		: std::is_same_v<ReturnType, ECS::ComponentType> ? ExpressionResultTypes::ComponentType
		: ExpressionResultTypes::TreeIdentifier;

	static_assert(k_returnType != ExpressionResultTypes::TreeIdentifier || std::is_same_v<ReturnType, TreeIdentifier>,
		"Cannot bind a function that does not return bool, double, ECS::ComponentType, or TreeIdentifier.");

	struct BindingFunctions
	{
		static ExpressionResult Call(
			const Interpreter& interpeter,
			void* untypedFunc,
			const Collection::Vector<ConditionAST::Expression>& expressions,
			const ECS::Entity& entity)
		{
			auto* func = static_cast<ReturnType(*)(const ECS::Entity&, ArgumentTypes...)>(untypedFunc);

			ExpressionResult evaluatedArguments[sizeof...(ArgumentTypes)]
			{
				interpreter.EvaluateExpression(expressions, entity)...
			};

			ReturnType result = func(entity,
				evaluatedArguments[Util::IndexOfType<ArgumentTypes, ArgumentTypes...>].Get<ArgumentTypes...>()...);

			return ExpressionResult::Make<ReturnType>(std::move(result));
		}
	};

	m_boundFunctions[functionNameHash] = BoundFunction(func, &BindingFunctions::Call, k_returnType);
}
}
