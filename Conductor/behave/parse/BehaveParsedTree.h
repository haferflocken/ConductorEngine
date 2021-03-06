#pragma once

#include <collection/Variant.h>
#include <collection/Vector.h>
#include <util/StringHash.h>

#include <string>

namespace Behave::Parse
{
struct Expression;

struct NodeExpression
{
	NodeExpression(std::string&& nodeName, Collection::Vector<Expression>&& arguments)
		: m_nodeName(std::move(nodeName))
		, m_nodeNameHash(Util::CalcHash(m_nodeName))
		, m_arguments(std::move(arguments))
	{}

	// A node name may be any string that is not a keyword, begins with a lowercase letter,
	// and consists only of letters and numbers.
	std::string m_nodeName;
	Util::StringHash m_nodeNameHash;
	Collection::Vector<Expression> m_arguments;
};

struct FunctionCallExpression
{
	FunctionCallExpression(std::string&& functionName, Collection::Vector<Expression>&& arguments)
		: m_functionName(std::move(functionName))
		, m_functionNameHash(Util::CalcHash(m_functionName))
		, m_arguments(std::move(arguments))
	{}

	// A function name may be any string that is not a keyword, begins with an uppercase letter,
	// and consists only of letters and numbers.
	std::string m_functionName;
	Util::StringHash m_functionNameHash;
	Collection::Vector<Expression> m_arguments;
};

struct IdentifierExpression
{
	explicit IdentifierExpression(std::string&& treeName)
		: m_treeName(treeName)
		, m_treeNameHash(Util::CalcHash(m_treeName))
	{}

	// A tree name may be any string that is not a keyword, begins with an uppercase letter,
	// and consists only of letters and numbers. Tree names may not be any function name that is in use.
	std::string m_treeName;
	Util::StringHash m_treeNameHash;
};

struct NumericLiteral
{
	explicit NumericLiteral(double val)
		: m_value(val)
	{}

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
		, m_typeHash(Util::CalcHash(m_typeString))
	{}

	std::string m_typeString;
	Util::StringHash m_typeHash;
};

using LiteralExpression = Collection::Variant<
	NumericLiteral, StringLiteral, ResultLiteral, BooleanLiteral, ComponentTypeLiteral>;

struct Expression final : public Collection::Variant<
	NodeExpression,
	FunctionCallExpression,
	IdentifierExpression,
	LiteralExpression>
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

struct ParsedTree
{
	std::string m_treeName;
	NodeExpression m_rootNode{ "", Collection::Vector<Expression>() };
};
}
