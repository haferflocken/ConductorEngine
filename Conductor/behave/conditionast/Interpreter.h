#pragma once

#include <behave/conditionast/ASTTypes.h>
#include <behave/conditionast/BoundFunction.h>
#include <behave/conditionast/ExpressionResultType.h>

#include <collection/Variant.h>
#include <collection/VectorMap.h>
#include <util/StringHash.h>

namespace Behave::Parse { struct Expression; }
namespace ECS { class Entity; }

namespace Behave::ConditionAST
{
/**
 * An interpreter that evaluates ConditionAST expressions.
 */
class Interpreter
{
public:
	// Compile a parsed expression into an executable expression.
	Expression Compile(const Parse::Expression& parsedExpression) const;

	// Evaluate a ConditionAST::Expression on the given entity.
	ExpressionResultType EvaluateExpression(const Expression& expression, const ECS::Entity& entity) const;

	// Binds a function so that it can be called with ConditionAST::Expressions as arguments.
	template <typename ReturnType, typename... ArgumentTypes>
	void BindFunction(const Util::StringHash functionNameHash,
		ReturnType(*func)(const ECS::Entity&, ArgumentTypes...));

private:
	Collection::VectorMap<Util::StringHash, BoundFunction> m_boundFunctions;
};
}

namespace Behave::ConditionAST
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
