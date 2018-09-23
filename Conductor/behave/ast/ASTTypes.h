#pragma once

#include <behave/ast/BoundFunction.h>
#include <behave/ast/ExpressionResultType.h>

#include <collection/Variant.h>
#include <ecs/ComponentType.h>

namespace Behave::AST
{
struct Expression;

struct ComponentTypeLiteralExpression
{
	ComponentTypeLiteralExpression(ECS::ComponentType componentType, std::string&& componentTypeString)
		: m_componentType(componentType)
		, m_componentTypeString(std::move(componentTypeString))
	{}

	ECS::ComponentType m_componentType;
	std::string m_componentTypeString;
};

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
	ComponentTypeLiteralExpression,
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

	ExpressionResultTypeString GetResultType() const
	{
		ExpressionResultTypeString result;
		Match(
			[&](const None&) { result = ExpressionResultTypeString::Make<void>(); },
			[&](const bool&) { result = ExpressionResultTypeString::Make<bool>(); },
			[&](const double&) { result = ExpressionResultTypeString::Make<double>(); },
			[&](const std::string&) { result = ExpressionResultTypeString::Make<std::string>(); },
			[&](const ComponentTypeLiteralExpression& componentType)
			{
				result = ExpressionResultTypeString(componentType.m_componentTypeString.c_str());
			},
			[&](const TreeIdentifier&) { result = ExpressionResultTypeString::Make<TreeIdentifier>(); },
			[&](const FunctionCallExpression& argumentFunctionCall)
			{
				result = argumentFunctionCall.m_boundFunction.GetReturnType();
			});
		return result;
	}
};
}
