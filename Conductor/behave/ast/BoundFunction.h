#pragma once

#include <behave/ast/ExpressionResultType.h>

namespace ECS
{
class Entity;
class EntityManager;
}

namespace Behave::AST
{
struct Expression;
class Interpreter;

/**
 * A function binding that allows a normal C++ function to take AST::Expressions as arguments.
 */
class BoundFunction
{
public:
	using BindingFunction = ExpressionResult(*)(const Interpreter&, void*,
		const Collection::Vector<Expression>&, ECS::EntityManager&, const ECS::Entity&);

	BoundFunction()
		: m_untypedFunc(nullptr)
		, m_binding(nullptr)
	{}

	BoundFunction(void* untypedFunc, BindingFunction bindingFunc, ExpressionResultTypeString returnType)
		: m_untypedFunc(untypedFunc)
		, m_binding(bindingFunc)
		, m_returnType(returnType)
	{}

	ExpressionResult operator()(
		const Interpreter& interpreter,
		const Collection::Vector<Expression>& arguments,
		ECS::EntityManager& entityManager,
		const ECS::Entity& entity) const
	{
		return m_binding(interpreter, m_untypedFunc, arguments, entityManager, entity);
	}

	const ExpressionResultTypeString& GetReturnType() const { return m_returnType; }

private:
	void* m_untypedFunc;
	BindingFunction m_binding;
	ExpressionResultTypeString m_returnType;
};
}
