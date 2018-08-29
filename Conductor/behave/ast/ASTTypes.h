#pragma once

#include <behave/ast/BoundFunction.h>
#include <behave/ast/ExpressionResultType.h>

#include <collection/Variant.h>
#include <ecs/ComponentType.h>

namespace Behave::AST
{
struct Expression;

struct FunctionCallExpression
{
	FunctionCallExpression(const BoundFunction& boundFunction, Collection::Vector<Expression>&& arguments)
		: m_boundFunction(boundFunction)
		, m_arguments(std::move(arguments))
	{}

	BoundFunction m_boundFunction;
	Collection::Vector<Expression> m_arguments;
};

struct Expression final : public Collection::Variant<
	None,
	bool,
	double,
	std::string,
	ECS::ComponentType,
	TreeIdentifier,
	FunctionCallExpression>
{
	using Variant::Variant;

	Expression(Variant&& v)
		: Variant(std::move(v))
	{}

	template <typename T, typename... Args>
	static Expression Make(Args&&... args)
	{
		return Expression(Variant::Make<T, Args...>(std::forward<Args>(args)...));
	}
};
}
