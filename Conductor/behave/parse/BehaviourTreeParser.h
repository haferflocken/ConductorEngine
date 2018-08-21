#pragma once

#include <behave/parse/BehaveParsedTree.h>

namespace Behave::Parse
{
struct SyntaxError
{
	explicit SyntaxError(const char* message, int32_t lineNumber, int32_t characterInLine)
		: m_message(message)
		, m_lineNumber(lineNumber)
		, m_characterInLine(characterInLine)
	{}

	explicit SyntaxError(std::string&& message, int32_t lineNumber, int32_t characterInLine)
		: m_message(std::move(message))
		, m_lineNumber(lineNumber)
		, m_characterInLine(characterInLine)
	{}

	std::string m_message;
	int32_t m_lineNumber;
	int32_t m_characterInLine;
};

using ParseResult = Collection::Variant<Collection::Vector<ParsedTree>, SyntaxError>;

namespace Parser
{
ParseResult ParseTrees(const char* const input);
}
}
