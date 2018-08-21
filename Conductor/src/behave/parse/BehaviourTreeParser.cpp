#include <behave/parse/BehaviourTreeParser.h>

#include <cctype>

namespace Behave::Parse
{
namespace Internal_Parser
{
enum class TokenType : uint8_t
{
	Invalid,
	IndentNewLine,
	DedentNewLine,
	NewLine,
	OpenParen,
	CloseParen,
	OpenCurly,
	CloseCurly,
	Comma,
	ComponentTypeName,
	Text
};

constexpr size_t k_maxTokenLength = 129;

struct Token
{
	TokenType m_type = TokenType::Invalid;
	char m_chars[k_maxTokenLength]{ '\0' };
};

// Newlines and indentation are not considered whitespace.
bool IsWhitespace(char c)
{
	return (c == ' ' || c == '\f' || c == '\v');
}

bool IsDelimiter(char c)
{
	return (c == ' ' || c == '\f' || c == '\v' || c == '\t' || c == '\r' || c == '\n')
		|| (c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == '$')
		|| (c == '\0');
}

void ParseNextToken(const char*& i, int64_t& currentIndent, Token& token)
{
	token.m_type = TokenType::Invalid;

	// Skip over whitespace characters.
	while (IsWhitespace(*i))
	{
		++i;
	}

	const char c = *i;
	if (c == '\0')
	{
		return;
	}

	// Parse the token.
	if (c == '\r' || c == '\n')
	{
		// Skip over whatever bizarre sequence of carriage returns and newlines the file uses.
		while (*i == '\r' || *i == '\n')
		{
			++i;
		}

		// Measure the indentation of the line.
		int64_t indent = 0;
		while (*i == '\t')
		{
			++indent;
			++i;
		}

		const int64_t indentationDifference = indent - currentIndent;
		token.m_chars[0] = static_cast<char>(indentationDifference);
		token.m_chars[1] = '\0';

		currentIndent = indent;

		if (indentationDifference == 0)
		{
			token.m_type = TokenType::NewLine;
		}
		else if (indentationDifference < 0)
		{
			token.m_type = TokenType::DedentNewLine;
		}
		else
		{
			token.m_type = TokenType::IndentNewLine;
		}
	}
	else if (c == '(')
	{
		token.m_type = TokenType::OpenParen;
		token.m_chars[0] = c;
		token.m_chars[1] = '\0';
	}
	else if (c == ')')
	{
		token.m_type = TokenType::CloseParen;
		token.m_chars[0] = c;
		token.m_chars[1] = '\0';
	}
	else if (c == '{')
	{
		token.m_type = TokenType::OpenCurly;
		token.m_chars[0] = c;
		token.m_chars[1] = '\0';
	}
	else if (c == '}')
	{
		token.m_type = TokenType::CloseCurly;
		token.m_chars[0] = c;
		token.m_chars[1] = '\0';
	}
	else if (c == ',')
	{
		token.m_type = TokenType::Comma;
		token.m_chars[0] = c;
		token.m_chars[1] = '\0';
	}
	else
	{
		if (c == '$')
		{
			token.m_type = TokenType::ComponentTypeName;
			++i;
		}
		else
		{
			token.m_type = TokenType::Text;
		}

		size_t index = 0;
		while (index < (k_maxTokenLength - 1) && !IsDelimiter(*i))
		{
			token.m_chars[index] = *i;
			++i;
			++index;
		}
		token.m_chars[index] = '\0';
	}
}

bool IsTreeNameToken(const Token& token)
{
	if (token.m_type != TokenType::Text)
	{
		return false;
	}

	if (!std::isupper(token.m_chars[0]))
	{
		return false;
	}

	for (size_t i = 1; i < k_maxTokenLength && token.m_chars[i] != '\0'; ++i)
	{
		if (!std::isalnum(static_cast<unsigned char>(token.m_chars[i])))
		{
			return false;
		}
	}

	return true;
}

using ParseExpressionResult = Collection::Variant<Expression, SyntaxError>;

ParseExpressionResult ParseExpression(const char*& inputIter, int64_t& currentIndent, Token& token)
{
	// TODO(behave) Recursively parse expressions.
	return ParseExpressionResult();
}
}

/**
 * An input contains one or more trees.
 *
 * Keywords:
 *   tree, success, failure, true, false
 *
 * Grammar:
 *   A tree is the keyword tree, followed by a tree name, followed by the tree's root node on the
 *   next line. The tree keyword must not be indented. The tree's root node must not be indented.
 */
ParseResult Parser::ParseTrees(const char* const input)
{
	using namespace Internal_Parser;

	Collection::Vector<ParsedTree> outTrees;

	const char* inputIter = input;
	int64_t indent = 0;
	Token token;

	do
	{
		// Parse the "tree" keyword.
		ParseNextToken(inputIter, indent, token);

		if (token.m_type != TokenType::Text)
		{
			return ParseResult::Make<SyntaxError>("Unexpected non-text token encountered: expected \"tree\".");
		}
		if (strcmp(token.m_chars, "tree") != 0)
		{
			std::string message = "Unexpected token encountered: expected \"tree\", got \"";
			message += token.m_chars;
			message += "\".";
			return ParseResult::Make<SyntaxError>(std::move(message));
		}

		// Parse the tree's name.
		ParseNextToken(inputIter, indent, token);

		if (!IsTreeNameToken(token))
		{
			return ParseResult::Make<SyntaxError>("Tree names must begin with an uppercase letter and consist only of "
				"alphanumeric characters.");
		}

		ParsedTree& parsedTree = outTrees.Emplace();
		parsedTree.m_treeName = token.m_chars;

		// Parse the tree's root node.
		ParseExpressionResult rootExpressionResult = ParseExpression(inputIter, indent, token);
		SyntaxError rootSyntaxError{ "" };

		rootExpressionResult.Match(
			[&](Expression& expression)
			{
				if (expression.m_variant.Is<NodeExpression>())
				{
					parsedTree.m_rootNode = std::move(expression.m_variant.Get<NodeExpression>());
				}
				else
				{
					rootSyntaxError.m_message = "A tree's root must always be a node expression.";
				}
			},
			[&](SyntaxError& syntaxError)
			{
				rootSyntaxError = std::move(syntaxError);
			});

		if (!rootSyntaxError.m_message.empty())
		{
			return ParseResult::Make<SyntaxError>(std::move(rootSyntaxError));
		}

	} while (token.m_type != TokenType::Invalid);

	return ParseResult::Make<Collection::Vector<ParsedTree>>(std::move(outTrees));
}
}
