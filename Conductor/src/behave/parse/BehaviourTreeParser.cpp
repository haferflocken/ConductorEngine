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
	StringLiteral,
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
		|| (c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == '$' || c == '"')
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
		++i;
	}
	else if (c == ')')
	{
		token.m_type = TokenType::CloseParen;
		token.m_chars[0] = c;
		token.m_chars[1] = '\0';
		++i;
	}
	else if (c == '{')
	{
		token.m_type = TokenType::OpenCurly;
		token.m_chars[0] = c;
		token.m_chars[1] = '\0';
		++i;
	}
	else if (c == '}')
	{
		token.m_type = TokenType::CloseCurly;
		token.m_chars[0] = c;
		token.m_chars[1] = '\0';
		++i;
	}
	else if (c == ',')
	{
		token.m_type = TokenType::Comma;
		token.m_chars[0] = c;
		token.m_chars[1] = '\0';
		++i;
	}
	else if (c == '"')
	{
		token.m_type = TokenType::StringLiteral;
		++i;

		size_t index = 0;
		while (index < (k_maxTokenLength - 1) && *i != '\0' && *i != '"')
		{
			token.m_chars[index] = *i;
			++i;
			++index;
		}
		token.m_chars[index] = '\0';

		if (*i == '"')
		{
			++i;
		}
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

enum class KeywordType : uint8_t
{
	Invalid,
	Tree,
	Success,
	Failure,
	True,
	False
};

KeywordType ConvertToKeyword(const char* const str)
{
	if (strcmp(str, "tree") == 0)
	{
		return KeywordType::Tree;
	}
	if (strcmp(str, "success") == 0)
	{
		return KeywordType::Success;
	}
	if (strcmp(str, "failure") == 0)
	{
		return KeywordType::Failure;
	}
	if (strcmp(str, "true") == 0)
	{
		return KeywordType::True;
	}
	if (strcmp(str, "false") == 0)
	{
		return KeywordType::False;
	}
	return KeywordType::Invalid;
}

bool IsTreeName(const char* const str)
{
	if (!std::isupper(str[0]))
	{
		return false;
	}

	for (size_t i = 1; i < k_maxTokenLength && str[i] != '\0'; ++i)
	{
		if (!std::isalnum(static_cast<unsigned char>(str[i])))
		{
			return false;
		}
	}

	return true;
}

bool IsNodeName(const char* const str)
{
	if (!std::islower(str[0]))
	{
		return false;
	}

	Dev::FatalAssert(ConvertToKeyword(str) == KeywordType::Invalid,
		"Keyword strings should not be passed into IsNodeName().");

	for (size_t i = 1; i < k_maxTokenLength && str[i] != '\0'; ++i)
	{
		if (!std::isalnum(static_cast<unsigned char>(str[i])))
		{
			return false;
		}
	}

	return true;
}

bool IsFunctionName(const char* const str)
{
	// A function name may be alphanumeric.
	if (std::isupper(str[0]))
	{
		for (size_t i = 1; i < k_maxTokenLength && str[i] != '\0'; ++i)
		{
			if (!std::isalnum(static_cast<unsigned char>(str[i])))
			{
				return false;
			}
		}

		return true;
	}

	// A function name may be a string of specific symbols.
	for (size_t i = 0; i < k_maxTokenLength && str[i] != '\0'; ++i)
	{
		const char c = str[i];
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

ParseExpressionResult MakeLiteralExpressionFromKeywordType(const KeywordType keywordType)
{
	switch (keywordType)
	{
	case KeywordType::Tree:
	{
		return ParseExpressionResult::Make<SyntaxError>("The \"tree\" keyword cannot be made into a literal.");
	}
	case KeywordType::Success:
	{
		auto result = ParseExpressionResult::Make<Expression>();
		Expression& expression = result.Get<Expression>();

		expression.m_variant = decltype(Expression::m_variant)::Make<LiteralExpression>();
		expression.m_variant.Get<LiteralExpression>() = LiteralExpression::Make<ResultLiteral>(true);

		return result;
	}
	case KeywordType::Failure:
	{
		auto result = ParseExpressionResult::Make<Expression>();
		Expression& expression = result.Get<Expression>();

		expression.m_variant = decltype(Expression::m_variant)::Make<LiteralExpression>();
		expression.m_variant.Get<LiteralExpression>() = LiteralExpression::Make<ResultLiteral>(false);

		return result;
	}
	case KeywordType::True:
	{
		auto result = ParseExpressionResult::Make<Expression>();
		Expression& expression = result.Get<Expression>();

		expression.m_variant = decltype(Expression::m_variant)::Make<LiteralExpression>();
		expression.m_variant.Get<LiteralExpression>() = LiteralExpression::Make<BooleanLiteral>(true);

		return result;
	}
	case KeywordType::False:
	{
		auto result = ParseExpressionResult::Make<Expression>();
		Expression& expression = result.Get<Expression>();

		expression.m_variant = decltype(Expression::m_variant)::Make<LiteralExpression>();
		expression.m_variant.Get<LiteralExpression>() = LiteralExpression::Make<BooleanLiteral>(false);

		return result;
	}
	default:
	{
		Dev::FatalError("Unrecognized keyword type [%d].", static_cast<int32_t>(keywordType));
		return ParseExpressionResult();
	}
	}
}

ParseExpressionResult MakeComponentTypeLiteralExpression(const Token& token)
{
	if (!IsComponentTypeNameToken(token))
	{
		return ParseExpressionResult::Make<SyntaxError>(
			"Component type names must consist only of alphanumeric characters and underscores.");
	}

	auto result = ParseExpressionResult::Make<Expression>();
	Expression& expression = result.Get<Expression>();

	expression.m_variant = decltype(Expression::m_variant)::Make<LiteralExpression>(
		LiteralExpression::Make<ComponentTypeLiteral>(token.m_chars));

	return result;
}

ParseExpressionResult MakeStringLiteralExpression(const Token& token)
{
	Dev::FatalAssert(token.m_type == TokenType::StringLiteral,
		"Only string literal tokens should be passed into MakeStringLiteralExpression().");

	auto result = ParseExpressionResult::Make<Expression>();
	Expression& expression = result.Get<Expression>();

	expression.m_variant = decltype(Expression::m_variant)::Make<LiteralExpression>();
	expression.m_variant.Get<LiteralExpression>() = LiteralExpression::Make<StringLiteral>(token.m_chars);

	return result;
}

ParseExpressionResult MakeNumericLiteralExpression(const Token& token)
{
	Dev::FatalAssert(token.m_type == TokenType::Text,
		"Only text tokens should be passed into MakeNumericLiteralExpression().");

	// TODO numeric literals

	return ParseExpressionResult::Make<SyntaxError>("NUMERIC LITERALS NOT YET SUPPORTED");
}

using ParseArgumentListResult = Collection::Variant<Collection::Vector<Expression>, SyntaxError>;

ParseArgumentListResult ParseSingleLineArgumentList(const char*& inputIter, int64_t& currentIndent, Token& token)
{
	ParseNextToken(inputIter, currentIndent, token);
	if (token.m_type != TokenType::OpenParen)
	{
		return ParseArgumentListResult::Make<SyntaxError>(
			"Argument lists within single-line expressions must be begin with an open parenthesis.");
	}

	Collection::Vector<Expression> arguments;

	const char* peekIter = inputIter;
	int64_t peekIndent = currentIndent;

	ParseNextToken(peekIter, peekIndent, token);
	while (token.m_type != TokenType::CloseParen)
	{
		auto argumentResult = ParseSingleLineExpression(inputIter, currentIndent, token);
		if (argumentResult.Is<SyntaxError>())
		{
			return ParseArgumentListResult::Make<SyntaxError>(std::move(argumentResult.Get<SyntaxError>()));
		}
		
		arguments.Add(std::move(argumentResult.Get<Expression>()));

		peekIter = inputIter;
		peekIndent = currentIndent;
		ParseNextToken(peekIter, peekIndent, token);
		if (token.m_type == TokenType::Comma)
		{
			inputIter = peekIter;
			currentIndent = peekIndent;
		}
		else if (token.m_type != TokenType::CloseParen)
		{
			return ParseArgumentListResult::Make<SyntaxError>(
				"Arguments within single-line expressions must be comma separated.");
		}
	}
	inputIter = peekIter;
	currentIndent = peekIndent;

	return ParseArgumentListResult::Make<Collection::Vector<Expression>>(std::move(arguments));
}

ParseExpressionResult ParseSingleLineNodeExpression(const char*& inputIter, int64_t& currentIndent, Token& token)
{
	std::string nodeName{ token.m_chars };

	ParseArgumentListResult argumentListResult = ParseSingleLineArgumentList(inputIter, currentIndent, token);
	if (argumentListResult.Is<SyntaxError>())
	{
		return ParseExpressionResult::Make<SyntaxError>(argumentListResult.Get<SyntaxError>());
	}

	auto result = ParseExpressionResult::Make<Expression>();
	Expression& expression = result.Get<Expression>();

	expression.m_variant = decltype(Expression::m_variant)::Make<NodeExpression>(
		std::move(nodeName), std::move(argumentListResult.Get<Collection::Vector<Expression>>()));

	return result;
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
	case TokenType::StringLiteral:
	{
		return MakeStringLiteralExpression(token);
	}
	case TokenType::Text:
	{
		// If this token is a keyword, there is no ambiguity.
		// If this token is a node name, there is no ambiguity.
		// There is ambiguity between function names and tree names. They can be differentiated
		// by the token following the name.
		// All other tokens are treated as numeric literals.
		const KeywordType keywordType = ConvertToKeyword(token.m_chars);
		if (keywordType != KeywordType::Invalid)
		{
			return MakeLiteralExpressionFromKeywordType(keywordType);
		}
		else if (IsNodeName(token.m_chars))
		{
			return ParseSingleLineNodeExpression(inputIter, currentIndent, token);
		}
		else if (IsFunctionName(token.m_chars))
		{
			std::string name{ token.m_chars };
			
			// If the next token is an open parenthesis, this is a function call expression.
			// Otherwise, this is a tree identifier.
			const char* peekIter = inputIter;
			int64_t peekIndent = currentIndent;
			ParseNextToken(peekIter, peekIndent, token);

			if (token.m_type == TokenType::OpenParen)
			{
				auto argumentListResult = ParseSingleLineArgumentList(inputIter, currentIndent, token);
				if (argumentListResult.Is<SyntaxError>())
				{
					return ParseExpressionResult::Make<SyntaxError>(std::move(argumentListResult.Get<SyntaxError>()));
				}

				auto result = ParseExpressionResult::Make<Expression>();
				Expression& expression = result.Get<Expression>();

				expression.m_variant = decltype(Expression::m_variant)::Make<FunctionCallExpression>(
					std::move(name), std::move(argumentListResult.Get<Collection::Vector<Expression>>()));

				return result;
			}
			else if (!IsTreeName(name.c_str()))
			{
				return ParseExpressionResult::Make<SyntaxError>(
					"Unexpected function name encountered; expected a tree name.");
			}
			else
			{
				auto result = ParseExpressionResult::Make<Expression>();
				Expression& expression = result.Get<Expression>();

				expression.m_variant = decltype(Expression::m_variant)::Make<IdentifierExpression>(std::move(name));

				return result;
			}
		}
		
		Dev::FatalAssert(!IsTreeName(token.m_chars), "All tree names should be handled by the function name condition.");

		return MakeNumericLiteralExpression(token);
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
	if (!IsFunctionName(token.m_chars))
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

	Collection::Vector<Expression> arguments;
	arguments.Add(std::move(leftExpressionResult.Get<Expression>()));
	arguments.Add(std::move(rightExpressionResult.Get<Expression>()));

	auto result = ParseExpressionResult::Make<Expression>();
	Expression& expression = result.Get<Expression>();

	expression.m_variant = decltype(Expression::m_variant)::Make<FunctionCallExpression>(
		std::move(functionName), std::move(arguments));

	return result;
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
	case TokenType::StringLiteral:
	{
		return MakeStringLiteralExpression(token);
	}
	case TokenType::Text:
	{
		// TODO text tokens
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
		if (token.m_type != TokenType::Text)
		{
			return ParseResult::Make<SyntaxError>("Unexpected non-text token encountered: expected a tree name.");
		}
		if (!IsTreeName(token.m_chars))
		{
			return ParseResult::Make<SyntaxError>(
				"Tree names must begin with an uppercase letter and consist only of alphanumeric characters.");
		}

		ParsedTree& parsedTree = outTrees.Emplace();
		parsedTree.m_treeName = token.m_chars;

		// Ensure there is a newline after the tree's name.
		ParseNextToken(inputIter, indent, token);
		if (token.m_type != TokenType::NewLine)
		{
			return ParseResult::Make<SyntaxError>(
				"A tree's root node must be on the line following the tree's name declaration.");
		}

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
