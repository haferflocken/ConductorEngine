#pragma once

#include <behave/conditionast/ExpressionResultType.h>

namespace ECS { class Entity; }

namespace Behave::ConditionAST
{
struct Expression;
class Interpreter;

/**
 * A function binding that allows a normal C++ function to take ConditionAST::Expressions as arguments.
 */
class BoundFunction
{
public:
	using BindingFunction = ExpressionResultType(*)(const Interpreter&, void*,
		const Collection::Vector<Expression>&, const ECS::Entity&);

	BoundFunction(void* untypedFunc, BindingFunction bindingFunc)
		: m_untypedFunc(untypedFunc)
		, m_binding(bindingFunc)
	{}

	ExpressionResultType operator()(
		const Interpreter& interpreter,
		const Collection::Vector<Expression>& arguments,
		const ECS::Entity& entity) const
	{
		return m_binding(interpreter, m_untypedFunc, arguments, entity);
	}

private:
	void* m_untypedFunc;
	BindingFunction m_binding;
};
}
