#pragma once

#include <collection/Variant.h>
#include <collection/Vector.h>

#include <string>

namespace Behave::Parse
{
struct Expression;

struct NodeExpression
{
	NodeExpression(std::string&& nodeName, Collection::Vector<Expression>&& arguments)
		: m_nodeName(std::move(nodeName))
		, m_arguments(std::move(arguments))
	{}

	// A node name may be any string that is not a keyword, begins with a lowercase letter,
	// and consists only of letters and numbers.
	std::string m_nodeName;
	Collection::Vector<Expression> m_arguments;
};

struct FunctionCallExpression
{
	FunctionCallExpression(std::string&& functionName, Collection::Vector<Expression>&& arguments)
		: m_functionName(std::move(functionName))
		, m_arguments(std::move(arguments))
	{}

	// A function name may be any string that is not a keyword, begins with an uppercase letter,
	// and consists only of letters and numbers.
	std::string m_functionName;
	Collection::Vector<Expression> m_arguments;
};

struct IdentifierExpression
{
	explicit IdentifierExpression(std::string&& treeName)
		: m_treeName(treeName)
	{}

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
	explicit StringLiteral(const char* vBegin, const char* vEnd)
		: m_value(vBegin, vEnd)
	{}

	std::string m_value;
};

struct ResultLiteral
{
	explicit ResultLiteral(bool isSuccess)
		: m_isSuccess(isSuccess)
	{}

	bool m_isSuccess;
};

struct BooleanLiteral
{
	explicit BooleanLiteral(bool v)
		: m_value(v)
	{}

	bool m_value;
};

struct ComponentTypeLiteral
{
	explicit ComponentTypeLiteral(const char* typeStringBegin, const char* typeStringEnd)
		: m_typeString(typeStringBegin, typeStringEnd)
	{}

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
	NodeExpression m_rootNode{ "", Collection::Vector<Expression>() };
};
}
