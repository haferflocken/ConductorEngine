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
/**
 * An interpreter that evaluates AST expressions.
 */
class Interpreter
{
public:
	// Compile a parsed expression into an executable expression.
	Expression Compile(const Parse::Expression& parsedExpression) const;

	// Evaluate an AST::Expression on the given entity.
	ExpressionResultType EvaluateExpression(const Expression& expression, const ECS::Entity& entity) const;

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
	struct BindingFunctions
	{
		static ExpressionResultType Call(
			const Interpreter& interpeter,
			void* untypedFunc,
			const Collection::Vector<ConditionAST::Expression>& expressions,
			const ECS::Entity& entity)
		{
			auto* func = static_cast<ReturnType(*)(const ECS::Entity&, ArgumentTypes...)>(untypedFunc);

			ExpressionResultType evaluatedArguments[sizeof...(ArgumentTypes)]
			{
				interpreter.EvaluateExpression(expressions, entity)...
			};

			ReturnType result = func(entity,
				evaluatedArguments[Util::IndexOfType<ArgumentTypes, ArgumentTypes...>].Get<ArgumentTypes...>()...);

			return ExpressionResultType::Make<ReturnType>(std::move(result));
		}
	};

	m_boundFunctions[functionNameHash] = BoundFunction(func, &BindingFunctions::Call);
}
}
