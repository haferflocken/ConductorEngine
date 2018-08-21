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

bool IsDelimiter(char c)
{
	return (c == ' ' || c == '\f' || c == '\v' || c == '\t' || c == '\r' || c == '\n')
		|| (c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == '$')
		|| (c == '\0');
}

void ParseNextToken(const char*& i, int64_t& currentIndent, Token& token)
{
	token.m_type = TokenType::Invalid;

	// Skip over whitespace characters and comments.
	while (true)
	{
		if (*i == ' ' || *i == '\f' || *i == '\v')
		{
			++i;
		}
		else if (*i == '#')
		{
			++i;
			while (*i != '\0' && *i != '\r' && *i != '\n')
			{
				++i;
			}
		}
		else
		{
			break;
		}
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

bool IsFunctionNameToken(const Token& token)
{
	if (token.m_type != TokenType::Text)
	{
		return false;
	}

	// A function name may be alphanumeric.
	if (std::isupper(token.m_chars[0]))
	{
		for (size_t i = 1; i < k_maxTokenLength && token.m_chars[i] != '\0'; ++i)
		{
			if (!std::isalnum(static_cast<unsigned char>(token.m_chars[i])))
			{
				return false;
			}
		}

		return true;
	}

	// A function name may be a string of specific symbols.
	for (size_t i = 0; i < k_maxTokenLength && token.m_chars[i] != '\0'; ++i)
	{
		const char c = token.m_chars[i];
		if (c != '<' && c != '>' && c != '=' && c != '+' && c != '-' && c != '*' && c != '/' && c != '^'
			&& c != '~' && c != '!' && c != '%')
		{
			return false;
		}
	}

	return true;
}

bool IsComponentTypeNameToken(const Token& token)
{
	if (token.m_type != TokenType::ComponentTypeName)
	{
		return false;
	}

	for (size_t i = 0; i < k_maxTokenLength && token.m_chars[i] != '\0'; ++i)
	{
		if ((!std::isalnum(static_cast<unsigned char>(token.m_chars[i])))
			&& (token.m_chars[i] != '_'))
		{
			return false;
		}
	}

	return true;
}

using ParseExpressionResult = Collection::Variant<Expression, SyntaxError>;
ParseExpressionResult ParseSingleLineExpression(const char*& inputIter, int64_t& currentIndent, Token& token);
ParseExpressionResult ParseInfixExpression(const char*& inputIter, int64_t& currentIndent, Token& token);
ParseExpressionResult ParseExpression(const char*& inputIter, int64_t& currentIndent, Token& token);

ParseExpressionResult MakeComponentTypeLiteralExpression(const Token& token)
{
	if (!IsComponentTypeNameToken(token))
	{
		return ParseExpressionResult::Make<SyntaxError>(
			"Component type names must consist only of alphanumeric characters and underscores.");
	}

	Expression expression;
	expression.m_variant = decltype(expression.m_variant)::Make<LiteralExpression>(
		LiteralExpression::Make<ComponentTypeLiteral>(token.m_chars));

	return ParseExpressionResult::Make<Expression>(std::move(expression));
}

// Parse an expression within a single line. Does not necessarily consume the entire line.
ParseExpressionResult ParseSingleLineExpression(const char*& inputIter, int64_t& currentIndent, Token& token)
{
	ParseNextToken(inputIter, currentIndent, token);
	
	switch (token.m_type)
	{
	case TokenType::Invalid:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected end of input encountered.");
	}
	case TokenType::IndentNewLine:
	case TokenType::DedentNewLine:
	case TokenType::NewLine:
	{
		return ParseExpressionResult::Make<SyntaxError>(
			"Unexpected new line encountered; expected expression to be on a single line.");
	}
	case TokenType::OpenParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '(' encountered.");
	}
	case TokenType::CloseParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ')' encountered.");
	}
	case TokenType::OpenCurly:
	{
		return ParseInfixExpression(inputIter, currentIndent, token);
	}
	case TokenType::CloseCurly:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '}' encountered.");
	}
	case TokenType::Comma:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ',' encountered.");
	}
	case TokenType::ComponentTypeName:
	{
		return MakeComponentTypeLiteralExpression(token);
	}
	case TokenType::Text:
	{
		// TODO

		return ParseExpressionResult();
	}
	default:
	{
		Dev::FatalError("Behave DSL parser error: unknown token type [%d].", static_cast<int32_t>(token.m_type));
		return ParseExpressionResult();
	}
	}
}

// An infix expression is a binary function call expression wrapped in curly braces where the function name
// is the middle token.
ParseExpressionResult ParseInfixExpression(const char*& inputIter, int64_t& currentIndent, Token& token)
{
	const ParseExpressionResult leftExpressionResult = ParseSingleLineExpression(inputIter, currentIndent, token);
	if (!leftExpressionResult.Is<Expression>())
	{
		return leftExpressionResult;
	}

	ParseNextToken(inputIter, currentIndent, token);
	if (token.m_type != TokenType::Text)
	{
		return ParseExpressionResult::Make<SyntaxError>(
			"Unexpected non-text token encountered; expected a function name.");
	}
	if (!IsFunctionNameToken(token))
	{
		std::string message = "Expected a function name; encountered \"";
		message += token.m_chars;
		message += "\".";

		return ParseExpressionResult::Make<SyntaxError>(std::move(message));
	}

	std::string functionName{ token.m_chars };

	const ParseExpressionResult rightExpressionResult = ParseSingleLineExpression(inputIter, currentIndent, token);
	if (!rightExpressionResult.Is<Expression>())
	{
		return rightExpressionResult;
	}

	Expression expression;
	expression.m_variant = decltype(expression.m_variant)::Make<FunctionCallExpression>();

	FunctionCallExpression& functionCallExpression = expression.m_variant.Get<FunctionCallExpression>();
	functionCallExpression.m_functionName = std::move(functionName);
	functionCallExpression.m_arguments.Add(std::move(leftExpressionResult.Get<Expression>()));
	functionCallExpression.m_arguments.Add(std::move(rightExpressionResult.Get<Expression>()));

	return ParseExpressionResult::Make<Expression>(std::move(expression));
}

ParseExpressionResult ParseExpression(const char*& inputIter, int64_t& currentIndent, Token& token)
{
	ParseNextToken(inputIter, currentIndent, token);
	
	switch (token.m_type)
	{
	case TokenType::Invalid:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected end of input encountered.");
	}
	case TokenType::IndentNewLine:
	{
		return ParseExpression(inputIter, currentIndent, token);
	}
	case TokenType::DedentNewLine:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected dedent encountered.");
	}
	case TokenType::NewLine:
	{
		return ParseExpression(inputIter, currentIndent, token);
	}
	case TokenType::OpenParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '(' encountered.");
	}
	case TokenType::CloseParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ')' encountered.");
	}
	case TokenType::OpenCurly:
	{
		return ParseInfixExpression(inputIter, currentIndent, token);
	}
	case TokenType::CloseCurly:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '}' encountered.");
	}
	case TokenType::Comma:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ',' encountered.");
	}
	case TokenType::ComponentTypeName:
	{
		return MakeComponentTypeLiteralExpression(token);
	}
	case TokenType::Text:
	{
		// TODO
		return ParseExpressionResult();
	}
	default:
	{
		Dev::FatalError("Behave DSL parser error: unknown token type [%d].", static_cast<int32_t>(token.m_type));
		return ParseExpressionResult();
	}
	}
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
 *   
 *   A line comment begins with # and continues to the end of that line.
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
			return ParseResult::Make<SyntaxError>(
				"Tree names must begin with an uppercase letter and consist only of alphanumeric characters.");
		}

		ParsedTree& parsedTree = outTrees.Emplace();
		parsedTree.m_treeName = token.m_chars;

		// Parse the tree's root node.
		ParseExpressionResult rootExpressionResult = ParseExpression(inputIter, indent, token);
		if (!rootExpressionResult.IsAny())
		{
			// This only happens due to internal parser errors and is not part of normal control flow.
			return ParseResult();
		}

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
