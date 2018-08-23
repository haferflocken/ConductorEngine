#pragma once

#include <behave/conditionast/BoundFunction.h>

#include <collection/Variant.h>
#include <ecs/ComponentType.h>
#include <util/StringHash.h>

namespace Behave::ConditionAST
{
struct Expression;

struct BooleanLiteralExpression
{
	bool m_value;
};

struct NumericLiteralExpression
{
	double m_value;
};

struct ComponentTypeLiteralExpression
{
	ECS::ComponentType m_type;
};

struct TreeIdentifierExpression
{
	Util::StringHash m_treeNameHash;
};

struct FunctionCallExpression
{
	BoundFunction m_boundFunction;
	Collection::Vector<Expression> m_arguments;
};

struct Expression
{
	Collection::Variant<
		BooleanLiteralExpression,
		NumericLiteralExpression,
		ComponentTypeLiteralExpression,
		TreeIdentifierExpression,
		FunctionCallExpression> m_variant;
};
}
