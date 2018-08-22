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
	DedentNewLine, // TODO separate dedents from their newlines to make multi-line argument lists work
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

struct Token
{
	TokenType m_type = TokenType::Invalid;
	const char* m_charsBegin{ nullptr };
	const char* m_charsEnd{ nullptr };
	int32_t m_lineNumber{ -1 };
	int32_t m_characterIndex{ -1 };
};

struct ParsingState
{
	const char* i;
	int64_t currentIndent;
	Token token;
	int32_t lineNumber;
	int32_t characterInLine;
};

bool IsDelimiter(char c)
{
	return (c == ' ' || c == '\f' || c == '\v' || c == '\t' || c == '\r' || c == '\n')
		|| (c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == '$' || c == '"')
		|| (c == '\0');
}

void ParseNextToken(ParsingState& state)
{
	const char*& i = state.i;
	int64_t& currentIndent = state.currentIndent;
	Token& token = state.token;

	token.m_type = TokenType::Invalid;

	// Skip over whitespace characters and comments.
	while (true)
	{
		if (*i == ' ' || *i == '\f' || *i == '\v')
		{
			++i;
			++state.characterInLine;
		}
		else if (*i == '#')
		{
			++i;
			++state.characterInLine;
			while (*i != '\0' && *i != '\r' && *i != '\n')
			{
				++i;
				++state.characterInLine;
			}
		}
		else
		{
			break;
		}
	}

	token.m_charsBegin = i;

	const char c = *i;
	if (c == '\0')
	{
		token.m_charsEnd = i;
		return;
	}

	// Parse the token.
	if (c == '\r' || c == '\n')
	{
		++i;
		if (c == '\r' && *i == '\n')
		{
			++i;
		}

		++state.lineNumber;
		state.characterInLine = 0;

		// Measure the indentation of the line.
		int64_t indent = 0;
		while (*i == '\t')
		{
			++indent;
			++i;
			++state.characterInLine;
		}
		token.m_charsEnd = i;

		const int64_t indentationDifference = indent - currentIndent;
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
		++i;
		++state.characterInLine;
		token.m_charsEnd = i;
	}
	else if (c == ')')
	{
		token.m_type = TokenType::CloseParen;
		++i;
		++state.characterInLine;
		token.m_charsEnd = i;
	}
	else if (c == '{')
	{
		token.m_type = TokenType::OpenCurly;
		++i;
		++state.characterInLine;
		token.m_charsEnd = i;
	}
	else if (c == '}')
	{
		token.m_type = TokenType::CloseCurly;
		++i;
		++state.characterInLine;
		token.m_charsEnd = i;
	}
	else if (c == ',')
	{
		token.m_type = TokenType::Comma;
		++i;
		++state.characterInLine;
		token.m_charsEnd = i;
	}
	else if (c == '"')
	{
		token.m_type = TokenType::StringLiteral;
		++i;
		++state.characterInLine;
		token.m_charsBegin = i;

		while (*i != '\0' && *i != '"')
		{
			++i;
			++state.characterInLine;
		}
		token.m_charsEnd = i;

		if (*i == '"')
		{
			++i;
			++state.characterInLine;
		}
	}
	else
	{
		if (c == '$')
		{
			token.m_type = TokenType::ComponentTypeName;
			++i;
			++state.characterInLine;
			token.m_charsBegin = i;
		}
		else
		{
			token.m_type = TokenType::Text;
		}

		while (!IsDelimiter(*i))
		{
			++i;
			++state.characterInLine;
		}
		token.m_charsEnd = i;
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

KeywordType ConvertToKeyword(const char* const strBegin, const char* const strEnd)
{
	const std::ptrdiff_t strLength = strEnd - strBegin;

	if (strLength == 4 && strncmp(strBegin, "tree", 4) == 0)
	{
		return KeywordType::Tree;
	}
	if (strLength == 7 &&  strncmp(strBegin, "success", 7) == 0)
	{
		return KeywordType::Success;
	}
	if (strLength == 7 && strncmp(strBegin, "failure", 7) == 0)
	{
		return KeywordType::Failure;
	}
	if (strLength == 4 && strncmp(strBegin, "true", 4) == 0)
	{
		return KeywordType::True;
	}
	if (strLength == 5 && strncmp(strBegin, "false", 5) == 0)
	{
		return KeywordType::False;
	}
	return KeywordType::Invalid;
}

bool IsTreeName(const char* const strBegin, const char* const strEnd)
{
	if (!std::isupper(strBegin[0]))
	{
		return false;
	}

	for (const char* i = strBegin + 1; i < strEnd; ++i)
	{
		if (!std::isalnum(static_cast<unsigned char>(*i)))
		{
			return false;
		}
	}

	return true;
}

bool IsNodeName(const char* const strBegin, const char* const strEnd)
{
	if (!std::islower(strBegin[0]))
	{
		return false;
	}

	Dev::FatalAssert(ConvertToKeyword(strBegin, strEnd) == KeywordType::Invalid,
		"Keyword strings should not be passed into IsNodeName().");

	for (const char* i = strBegin + 1; i < strEnd; ++i)
	{
		if (!std::isalnum(static_cast<unsigned char>(*i)))
		{
			return false;
		}
	}

	return true;
}

bool IsFunctionName(const char* const strBegin, const char* const strEnd)
{
	// A function name may be alphanumeric.
	if (std::isupper(strBegin[0]))
	{
		for (const char* i = strBegin + 1; i < strEnd; ++i)
		{
			if (!std::isalnum(static_cast<unsigned char>(*i)))
			{
				return false;
			}
		}
		return true;
	}

	// A function name may be a string of specific symbols.
	for (const char* i = strBegin; i < strEnd; ++i)
	{
		const char c = *i;
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

	for (const char* i = token.m_charsBegin; i < token.m_charsEnd; ++i)
	{
		if ((!std::isalnum(static_cast<unsigned char>(*i)))
			&& (*i != '_'))
		{
			return false;
		}
	}

	return true;
}

using ParseExpressionResult = Collection::Variant<Expression, SyntaxError>;
ParseExpressionResult ParseSingleLineExpression(ParsingState& state);
ParseExpressionResult ParseInfixExpression(ParsingState& state);
ParseExpressionResult ParseExpression(ParsingState& state);

ParseExpressionResult MakeLiteralExpressionFromKeywordType(const ParsingState& state, const KeywordType keywordType)
{
	switch (keywordType)
	{
	case KeywordType::Tree:
	{
		return ParseExpressionResult::Make<SyntaxError>("The \"tree\" keyword cannot be made into a literal.",
			state.lineNumber, state.characterInLine);
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

ParseExpressionResult MakeComponentTypeLiteralExpression(const ParsingState& state)
{
	if (!IsComponentTypeNameToken(state.token))
	{
		return ParseExpressionResult::Make<SyntaxError>(
			"Component type names must consist only of alphanumeric characters and underscores.",
			state.lineNumber, state.characterInLine);
	}

	auto result = ParseExpressionResult::Make<Expression>();
	Expression& expression = result.Get<Expression>();

	expression.m_variant = decltype(Expression::m_variant)::Make<LiteralExpression>(
		LiteralExpression::Make<ComponentTypeLiteral>(state.token.m_charsBegin, state.token.m_charsEnd));

	return result;
}

ParseExpressionResult MakeStringLiteralExpression(const ParsingState& state)
{
	Dev::FatalAssert(state.token.m_type == TokenType::StringLiteral,
		"Only string literal tokens should be passed into MakeStringLiteralExpression().");

	auto result = ParseExpressionResult::Make<Expression>();
	Expression& expression = result.Get<Expression>();

	expression.m_variant = decltype(Expression::m_variant)::Make<LiteralExpression>();
	expression.m_variant.Get<LiteralExpression>() = LiteralExpression::Make<StringLiteral>(
		state.token.m_charsBegin, state.token.m_charsEnd);

	return result;
}

ParseExpressionResult MakeNumericLiteralExpression(const ParsingState& state)
{
	Dev::FatalAssert(state.token.m_type == TokenType::Text,
		"Only text tokens should be passed into MakeNumericLiteralExpression().");

	// TODO numeric literals

	return ParseExpressionResult::Make<SyntaxError>("NUMERIC LITERALS NOT YET SUPPORTED",
		state.lineNumber, state.characterInLine);
}

using ParseArgumentListResult = Collection::Variant<Collection::Vector<Expression>, SyntaxError>;

ParseArgumentListResult ParseArgumentListHelper(ParsingState& state,
	const bool singleLineOnly, const TokenType separatorTokenType, const TokenType closingTokenType)
{
	Collection::Vector<Expression> arguments;

	ParsingState peekState = state;
	ParseNextToken(peekState);
	while (peekState.token.m_type != closingTokenType)
	{
		auto argumentResult = singleLineOnly ? ParseSingleLineExpression(state) : ParseExpression(state);
		if (argumentResult.Is<SyntaxError>())
		{
			return ParseArgumentListResult::Make<SyntaxError>(std::move(argumentResult.Get<SyntaxError>()));
		}

		arguments.Add(std::move(argumentResult.Get<Expression>()));

		peekState = state;
		ParseNextToken(peekState);
		if (peekState.token.m_type == separatorTokenType)
		{
			state = peekState;
		}
		else if (peekState.token.m_type != closingTokenType)
		{
			return ParseArgumentListResult::Make<SyntaxError>("Arguments must be comma separated "
				"(in single line argument lists) or new line separated (in multi line argument lists).",
				state.lineNumber, state.characterInLine);
		}
	}
	state = peekState;

	return ParseArgumentListResult::Make<Collection::Vector<Expression>>(std::move(arguments));
}

ParseArgumentListResult ParseArgumentList(ParsingState& state)
{
	ParseNextToken(state);
	if (state.token.m_type != TokenType::OpenParen
		&& state.token.m_type != TokenType::IndentNewLine)
	{
		return ParseArgumentListResult::Make<SyntaxError>(
			"Argument lists must be begin with an open parenthesis or an indented new line.",
			state.lineNumber, state.characterInLine);
	}
	const TokenType separatorTokenType =
		(state.token.m_type == TokenType::OpenParen) ? TokenType::Comma : TokenType::NewLine;
	const TokenType closingTokenType =
		(state.token.m_type == TokenType::OpenParen) ? TokenType::CloseParen : TokenType::DedentNewLine;

	return ParseArgumentListHelper(state, false, separatorTokenType, closingTokenType);
}

ParseArgumentListResult ParseSingleLineArgumentList(ParsingState& state)
{
	ParseNextToken(state);
	if (state.token.m_type != TokenType::OpenParen)
	{
		return ParseArgumentListResult::Make<SyntaxError>(
			"Argument lists within single-line expressions must begin with an open parenthesis.",
			state.lineNumber, state.characterInLine);
	}

	return ParseArgumentListHelper(state, true, TokenType::Comma, TokenType::CloseParen);
}

ParseExpressionResult ParseNodeExpression(ParsingState& state)
{
	std::string nodeName{ state.token.m_charsBegin, state.token.m_charsEnd };

	ParseArgumentListResult argumentListResult = ParseArgumentList(state);
	if (argumentListResult.Is<SyntaxError>())
	{
		return ParseExpressionResult::Make<SyntaxError>(std::move(argumentListResult.Get<SyntaxError>()));
	}

	auto result = ParseExpressionResult::Make<Expression>();
	Expression& expression = result.Get<Expression>();

	expression.m_variant = decltype(Expression::m_variant)::Make<NodeExpression>(
		std::move(nodeName), std::move(argumentListResult.Get<Collection::Vector<Expression>>()));

	return result;
}

ParseExpressionResult ParseSingleLineNodeExpression(ParsingState& state)
{
	std::string nodeName{ state.token.m_charsBegin, state.token.m_charsEnd };

	ParseArgumentListResult argumentListResult = ParseSingleLineArgumentList(state);
	if (argumentListResult.Is<SyntaxError>())
	{
		return ParseExpressionResult::Make<SyntaxError>(std::move(argumentListResult.Get<SyntaxError>()));
	}

	auto result = ParseExpressionResult::Make<Expression>();
	Expression& expression = result.Get<Expression>();

	expression.m_variant = decltype(Expression::m_variant)::Make<NodeExpression>(
		std::move(nodeName), std::move(argumentListResult.Get<Collection::Vector<Expression>>()));

	return result;
}

// Parse an expression within a single line. Does not necessarily consume the entire line.
ParseExpressionResult ParseSingleLineExpression(ParsingState& state)
{
	ParseNextToken(state);
	
	switch (state.token.m_type)
	{
	case TokenType::Invalid:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected end of input encountered.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::IndentNewLine:
	case TokenType::DedentNewLine:
	case TokenType::NewLine:
	{
		return ParseExpressionResult::Make<SyntaxError>(
			"Unexpected new line encountered; expected expression to be on a single line.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::OpenParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '(' encountered.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::CloseParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ')' encountered.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::OpenCurly:
	{
		return ParseInfixExpression(state);
	}
	case TokenType::CloseCurly:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '}' encountered.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::Comma:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ',' encountered.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::ComponentTypeName:
	{
		return MakeComponentTypeLiteralExpression(state);
	}
	case TokenType::StringLiteral:
	{
		return MakeStringLiteralExpression(state);
	}
	case TokenType::Text:
	{
		// If this token is a keyword, there is no ambiguity.
		// If this token is a node name, there is no ambiguity.
		// There is ambiguity between function names and tree names. They can be differentiated
		// by the token following the name.
		// All other tokens are treated as numeric literals.
		const KeywordType keywordType = ConvertToKeyword(state.token.m_charsBegin, state.token.m_charsEnd);
		if (keywordType != KeywordType::Invalid)
		{
			return MakeLiteralExpressionFromKeywordType(state, keywordType);
		}
		else if (IsNodeName(state.token.m_charsBegin, state.token.m_charsEnd))
		{
			return ParseSingleLineNodeExpression(state);
		}
		else if (IsFunctionName(state.token.m_charsBegin, state.token.m_charsEnd))
		{
			std::string name{ state.token.m_charsBegin, state.token.m_charsEnd };
			
			// If the next token is an open parenthesis, this is a function call expression.
			// Otherwise, this is a tree identifier.
			ParsingState peekState = state;
			ParseNextToken(peekState);

			if (peekState.token.m_type == TokenType::OpenParen)
			{
				auto argumentListResult = ParseSingleLineArgumentList(state);
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
			else if (!IsTreeName(name.data(), name.data() + name.length()))
			{
				return ParseExpressionResult::Make<SyntaxError>(
					"Unexpected function name encountered; expected a tree name.",
					state.lineNumber, state.characterInLine);
			}
			else
			{
				auto result = ParseExpressionResult::Make<Expression>();
				Expression& expression = result.Get<Expression>();

				expression.m_variant = decltype(Expression::m_variant)::Make<IdentifierExpression>(std::move(name));

				return result;
			}
		}
		
		Dev::FatalAssert(!IsTreeName(state.token.m_charsBegin, state.token.m_charsEnd),
			"All tree names should be handled by the function name condition.");

		return MakeNumericLiteralExpression(state);
	}
	default:
	{
		Dev::FatalError("Behave DSL parser error: unknown token type [%d].", static_cast<int32_t>(state.token.m_type));
		return ParseExpressionResult();
	}
	}
}

// An infix expression is a binary function call expression wrapped in curly braces where the function name
// is the middle token.
ParseExpressionResult ParseInfixExpression(ParsingState& state)
{
	const ParseExpressionResult leftExpressionResult = ParseSingleLineExpression(state);
	if (!leftExpressionResult.Is<Expression>())
	{
		return leftExpressionResult;
	}

	ParseNextToken(state);
	if (state.token.m_type != TokenType::Text)
	{
		return ParseExpressionResult::Make<SyntaxError>(
			"Unexpected non-text token encountered; expected a function name.",
			state.lineNumber, state.characterInLine);
	}
	if (!IsFunctionName(state.token.m_charsBegin, state.token.m_charsEnd))
	{
		std::string message = "Expected a function name; encountered \"";
		message += std::string(state.token.m_charsBegin, state.token.m_charsEnd);
		message += "\".";

		return ParseExpressionResult::Make<SyntaxError>(std::move(message), state.lineNumber, state.characterInLine);
	}

	std::string functionName{ state.token.m_charsBegin, state.token.m_charsEnd };

	const ParseExpressionResult rightExpressionResult = ParseSingleLineExpression(state);
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

ParseExpressionResult ParseExpression(ParsingState& state)
{
	ParseNextToken(state);
	
	switch (state.token.m_type)
	{
	case TokenType::Invalid:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected end of input encountered.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::IndentNewLine:
	{
		return ParseExpression(state);
	}
	case TokenType::DedentNewLine:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected dedent encountered.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::NewLine:
	{
		return ParseExpression(state);
	}
	case TokenType::OpenParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '(' encountered.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::CloseParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ')' encountered.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::OpenCurly:
	{
		return ParseInfixExpression(state);
	}
	case TokenType::CloseCurly:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '}' encountered.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::Comma:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ',' encountered.",
			state.lineNumber, state.characterInLine);
	}
	case TokenType::ComponentTypeName:
	{
		return MakeComponentTypeLiteralExpression(state);
	}
	case TokenType::StringLiteral:
	{
		return MakeStringLiteralExpression(state);
	}
	case TokenType::Text:
	{
		// If this token is a keyword, there is no ambiguity.
		// If this token is a node name, there is no ambiguity.
		// There is ambiguity between function names and tree names. They can be differentiated
		// by the token following the name.
		// All other tokens are treated as numeric literals.
		const KeywordType keywordType = ConvertToKeyword(state.token.m_charsBegin, state.token.m_charsEnd);
		if (keywordType != KeywordType::Invalid)
		{
			return MakeLiteralExpressionFromKeywordType(state, keywordType);
		}
		else if (IsNodeName(state.token.m_charsBegin, state.token.m_charsEnd))
		{
			return ParseNodeExpression(state);
		}
		else if (IsFunctionName(state.token.m_charsBegin, state.token.m_charsEnd))
		{
			std::string name{ state.token.m_charsBegin, state.token.m_charsEnd };

			// If the next token is an open parenthesis or an indenting newline, this is a function call expression.
			// Otherwise, this is a tree identifier.
			ParsingState peekState = state;
			ParseNextToken(peekState);

			if (peekState.token.m_type == TokenType::OpenParen
				|| peekState.token.m_type == TokenType::IndentNewLine)
			{
				auto argumentListResult = ParseArgumentList(state);
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
			else if (!IsTreeName(name.data(), name.data() + name.length()))
			{
				return ParseExpressionResult::Make<SyntaxError>(
					"Unexpected function name encountered; expected a tree name.",
					state.lineNumber, state.characterInLine);
			}
			else
			{
				auto result = ParseExpressionResult::Make<Expression>();
				Expression& expression = result.Get<Expression>();

				expression.m_variant = decltype(Expression::m_variant)::Make<IdentifierExpression>(std::move(name));

				return result;
			}
		}

		Dev::FatalAssert(!IsTreeName(state.token.m_charsBegin, state.token.m_charsEnd),
			"All tree names should be handled by the function name condition.");

		return MakeNumericLiteralExpression(state);
	}
	default:
	{
		Dev::FatalError("Behave DSL parser error: unknown token type [%d].", static_cast<int32_t>(state.token.m_type));
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

	ParsingState state;
	state.i = input;
	state.currentIndent = 0;
	state.lineNumber = 1;
	state.characterInLine = 0;

	do
	{
		// Parse the "tree" keyword, skipping blank lines between trees.
		ParseNextToken(state);
		if (state.token.m_type == TokenType::Invalid)
		{
			break;
		}
		if (state.token.m_type == TokenType::NewLine)
		{
			continue;
		}

		if (state.token.m_type != TokenType::Text)
		{
			return ParseResult::Make<SyntaxError>("Unexpected non-text token encountered: expected \"tree\".",
				state.lineNumber, state.characterInLine);
		}

		const std::ptrdiff_t tokenLength = state.token.m_charsEnd - state.token.m_charsBegin;
		if (tokenLength != 4 || strncmp(state.token.m_charsBegin, "tree", 4) != 0)
		{
			std::string message = "Unexpected token encountered: expected \"tree\", got \"";
			message += std::string(state.token.m_charsBegin, state.token.m_charsEnd);
			message += "\".";
			return ParseResult::Make<SyntaxError>(std::move(message), state.lineNumber, state.characterInLine);
		}

		// Parse the tree's name.
		ParseNextToken(state);
		if (state.token.m_type != TokenType::Text)
		{
			return ParseResult::Make<SyntaxError>("Unexpected non-text token encountered: expected a tree name.",
				state.lineNumber, state.characterInLine);
		}
		if (!IsTreeName(state.token.m_charsBegin, state.token.m_charsEnd))
		{
			return ParseResult::Make<SyntaxError>(
				"Tree names must begin with an uppercase letter and consist only of alphanumeric characters.",
				state.lineNumber, state.characterInLine);
		}

		ParsedTree& parsedTree = outTrees.Emplace();
		parsedTree.m_treeName = std::string(state.token.m_charsBegin, state.token.m_charsEnd);

		// Ensure there is a newline after the tree's name.
		ParseNextToken(state);
		if (state.token.m_type != TokenType::NewLine)
		{
			return ParseResult::Make<SyntaxError>(
				"A tree's root node must be on the line following the tree's name declaration.",
				state.lineNumber, state.characterInLine);
		}

		// Parse the tree's root node.
		ParseExpressionResult rootExpressionResult = ParseExpression(state);
		if (!rootExpressionResult.IsAny())
		{
			// This only happens due to internal parser errors and is not part of normal control flow.
			return ParseResult();
		}

		SyntaxError rootSyntaxError{ "", state.lineNumber, state.characterInLine };

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

	} while (state.token.m_type != TokenType::Invalid);

	return ParseResult::Make<Collection::Vector<ParsedTree>>(std::move(outTrees));
}
}
