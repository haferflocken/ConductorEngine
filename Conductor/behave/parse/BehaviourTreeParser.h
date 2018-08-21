#pragma once

#include <behave/parse/BehaveParsedTree.h>

namespace Behave::Parse
{
struct SyntaxError
{
	explicit SyntaxError(const char* message)
		: m_message(message)
	{}

	explicit SyntaxError(std::string&& message)
		: m_message(std::move(message))
	{}

	std::string m_message;
};

using ParseResult = Collection::Variant<Collection::Vector<ParsedTree>, SyntaxError>;

namespace Parser
{
ParseResult ParseTrees(const char* const input);
}
}
