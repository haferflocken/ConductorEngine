#pragma once

#include <behave/ast/BoundFunction.h>

#include <collection/Variant.h>
#include <ecs/ComponentType.h>
#include <util/StringHash.h>

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

struct Expression
{
	Collection::Variant<
		bool,
		double,
		std::string,
		ECS::ComponentType,
		TreeIdentifier,
		FunctionCallExpression> m_variant;
};
}
