#pragma once

#include <collection/Variant.h>
#include <collection/Vector.h>

#include <string>

namespace Behave::Parse
{
struct Expression;

struct NodeExpression
{
	// A node name may be any string that is not a keyword, begins with a lowercase letter,
	// and consists only of letters and numbers.
	std::string m_nodeName;
	Collection::Vector<Expression> m_arguments;
};

struct FunctionCallExpression
{
	// A function name may be any string that is not a keyword, begins with an uppercase letter,
	// and consists only of letters and numbers.
	std::string m_functionName;
	Collection::Vector<Expression> m_arguments;
};

struct IdentifierExpression
{
	// A tree name may be any string that is not a keyword, begins with an uppercase letter,
	// and consists only of letters and numbers. Tree names may not be any function name that is in use.
	std::string m_treeName;
};

struct NumericLiteral
{
	double m_value;
};

struct StringLiteral
{
	std::string m_value;
};

struct ResultLiteral
{
	bool m_isSuccess;
};

struct BooleanLiteral
{
	bool m_value;
};

struct ComponentTypeLiteral
{
	std::string m_typeString;
};

using LiteralExpression = Collection::Variant<
	NumericLiteral, StringLiteral, ResultLiteral, BooleanLiteral, ComponentTypeLiteral>;

struct Expression
{
	Collection::Variant<
		NodeExpression,
		FunctionCallExpression,
		IdentifierExpression,
		LiteralExpression> m_variant;
};

struct ParsedTree
{
	std::string m_treeName;
	NodeExpression m_rootNode;
};
}
