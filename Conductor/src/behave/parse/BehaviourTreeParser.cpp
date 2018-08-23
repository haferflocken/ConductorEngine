#include <behave/parse/BehaviourTreeParser.h>

#include <cctype>
#include <charconv>

namespace Behave::Parse
{
namespace Internal_Parser
{
enum class TokenType : uint8_t
{
	Indent,
	Dedent,
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
	Token(TokenType tokenType, const char* charsBegin, int32_t lineNumber, int32_t characterInLine)
		: m_type(tokenType)
		, m_charsBegin(charsBegin)
		, m_charsEnd(nullptr)
		, m_lineNumber(lineNumber)
		, m_characterInLine(characterInLine)
	{}

	TokenType m_type;
	const char* m_charsBegin;
	const char* m_charsEnd;
	int32_t m_lineNumber;
	int32_t m_characterInLine;
};

struct TokenizingState
{
	const char* i;
	int64_t currentIndent;
	int32_t lineNumber;
	int32_t characterInLine;
};

class ParsingState
{
	const Collection::Vector<Token>& m_tokens;
	int64_t m_tokenIndex;

public:
	explicit ParsingState(const Collection::Vector<Token>& tokens)
		: m_tokens(tokens)
		, m_tokenIndex(-1)
	{}

	bool HasMoreTokens() const { return (m_tokenIndex + 1) < m_tokens.Size(); }
	const Token& GetCurrentToken() const { return m_tokens[m_tokenIndex]; }
	const Token& PeekNextToken() const { return m_tokens[m_tokenIndex + 1]; }

	void Increment() { ++m_tokenIndex; }
};

bool IsDelimiter(char c)
{
	return (c == ' ' || c == '\f' || c == '\v' || c == '\t' || c == '\r' || c == '\n')
		|| (c == '(' || c == ')' || c == '{' || c == '}' || c == ',' || c == '$' || c == '"')
		|| (c == '\0');
}

bool ParseNextTokens(TokenizingState& state, Collection::Vector<Token>& outTokens)
{
	const char*& i = state.i;
	int64_t& currentIndent = state.currentIndent;

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

	const char c = *i;
	if (c == '\0')
	{
		return false;
	}

	// Parse the next tokens.
	if (c == '\r' || c == '\n')
	{
		Token newLineToken{ TokenType::NewLine, i, state.lineNumber, state.characterInLine };

		++i;
		if (c == '\r' && *i == '\n')
		{
			++i;
		}

		++state.lineNumber;
		state.characterInLine = 0;
		newLineToken.m_charsEnd = i;

		// Measure the indentation of the line.
		int64_t indent = 0;
		while (*i == '\t')
		{
			++indent;
			++i;
			++state.characterInLine;
		}
		
		const int64_t indentationDifference = indent - currentIndent;
		currentIndent = indent;

		if (indentationDifference < 0)
		{	
			for (int64_t j = 0; j > indentationDifference; --j)
			{
				outTokens.Emplace(TokenType::Dedent, nullptr, state.lineNumber, state.characterInLine);
			}
			outTokens.Add(newLineToken);
		}
		else if (indentationDifference > 0)
		{
			outTokens.Add(newLineToken);
			for (int64_t j = 0; j < indentationDifference; ++j)
			{
				outTokens.Emplace(TokenType::Indent, nullptr, state.lineNumber, state.characterInLine);
			}
		}
		else
		{
			outTokens.Add(newLineToken);
		}
	}
	else if (c == '(')
	{
		Token& token = outTokens.Emplace(TokenType::OpenParen, i, state.lineNumber, state.characterInLine);
		++i;
		++state.characterInLine;
		token.m_charsEnd = i;
	}
	else if (c == ')')
	{
		Token& token = outTokens.Emplace(TokenType::CloseParen, i, state.lineNumber, state.characterInLine);
		++i;
		++state.characterInLine;
		token.m_charsEnd = i;
	}
	else if (c == '{')
	{
		Token& token = outTokens.Emplace(TokenType::OpenCurly, i, state.lineNumber, state.characterInLine);
		++i;
		++state.characterInLine;
		token.m_charsEnd = i;
	}
	else if (c == '}')
	{
		Token& token = outTokens.Emplace(TokenType::CloseCurly, i, state.lineNumber, state.characterInLine);
		++i;
		++state.characterInLine;
		token.m_charsEnd = i;
	}
	else if (c == ',')
	{
		Token& token = outTokens.Emplace(TokenType::Comma, i, state.lineNumber, state.characterInLine);
		++i;
		++state.characterInLine;
		token.m_charsEnd = i;
	}
	else if (c == '"')
	{
		Token& token = outTokens.Emplace(TokenType::StringLiteral, i, state.lineNumber, state.characterInLine);
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
		Token& token = outTokens.Emplace(TokenType::ComponentTypeName, i, state.lineNumber, state.characterInLine);
		if (c == '$')
		{
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
	
	return true;
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
			state.GetCurrentToken().m_lineNumber, state.GetCurrentToken().m_characterInLine);
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
			"Component type names must consist only of alphanumeric characters and underscores.",
			token.m_lineNumber, token.m_characterInLine);
	}

	auto result = ParseExpressionResult::Make<Expression>();
	Expression& expression = result.Get<Expression>();

	expression.m_variant = decltype(Expression::m_variant)::Make<LiteralExpression>(
		LiteralExpression::Make<ComponentTypeLiteral>(token.m_charsBegin, token.m_charsEnd));

	return result;
}

ParseExpressionResult MakeStringLiteralExpression(const Token& token)
{
	Dev::FatalAssert(token.m_type == TokenType::StringLiteral,
		"Only string literal tokens should be passed into MakeStringLiteralExpression().");

	auto result = ParseExpressionResult::Make<Expression>();
	Expression& expression = result.Get<Expression>();

	expression.m_variant = decltype(Expression::m_variant)::Make<LiteralExpression>();
	expression.m_variant.Get<LiteralExpression>() = LiteralExpression::Make<StringLiteral>(
		token.m_charsBegin, token.m_charsEnd);

	return result;
}

ParseExpressionResult MakeNumericLiteralExpression(const Token& token)
{
	Dev::FatalAssert(token.m_type == TokenType::Text,
		"Only text tokens should be passed into MakeNumericLiteralExpression().");

	double val;
	std::from_chars_result fromCharsResult = std::from_chars(token.m_charsBegin, token.m_charsEnd, val);
	if (fromCharsResult.ptr != token.m_charsEnd)
	{
		std::string message = "Failed to parse \"";
		message += std::string(token.m_charsBegin, token.m_charsEnd);
		message += "\" as a number.";

		return ParseExpressionResult::Make<SyntaxError>(std::move(message),
			token.m_lineNumber, token.m_characterInLine);
	}

	auto result = ParseExpressionResult::Make<Expression>();
	Expression& expression = result.Get<Expression>();

	expression.m_variant = decltype(Expression::m_variant)::Make<LiteralExpression>();
	expression.m_variant.Get<LiteralExpression>() = LiteralExpression::Make<NumericLiteral>(val);

	return result;
}

using ParseArgumentListResult = Collection::Variant<Collection::Vector<Expression>, SyntaxError>;

ParseArgumentListResult ParseArgumentListHelper(ParsingState& state,
	const bool singleLineOnly, const TokenType separatorTokenType, const TokenType closingTokenType)
{
	Collection::Vector<Expression> arguments;

	while (state.HasMoreTokens() && state.PeekNextToken().m_type != closingTokenType)
	{
		auto argumentResult = singleLineOnly ? ParseSingleLineExpression(state) : ParseExpression(state);
		if (argumentResult.Is<SyntaxError>())
		{
			return ParseArgumentListResult::Make<SyntaxError>(std::move(argumentResult.Get<SyntaxError>()));
		}

		arguments.Add(std::move(argumentResult.Get<Expression>()));

		if (!state.HasMoreTokens())
		{
			return ParseArgumentListResult::Make<SyntaxError>("Unexpected end of input encountered.",
				state.GetCurrentToken().m_lineNumber, state.GetCurrentToken().m_characterInLine);
		}
		const Token& peekToken = state.PeekNextToken();

		if (peekToken.m_type == separatorTokenType)
		{
			state.Increment();
		}
		else if (peekToken.m_type != closingTokenType)
		{
			return ParseArgumentListResult::Make<SyntaxError>("Arguments must be comma separated "
				"(in single line argument lists) or new line separated (in multi line argument lists).",
				state.PeekNextToken().m_lineNumber, state.PeekNextToken().m_characterInLine);
		}
	}
	if (state.HasMoreTokens())
	{
		state.Increment();
	}
	
	return ParseArgumentListResult::Make<Collection::Vector<Expression>>(std::move(arguments));
}

ParseArgumentListResult ParseArgumentList(ParsingState& state)
{
	if (!state.HasMoreTokens())
	{
		return ParseArgumentListResult::Make<SyntaxError>("Unexpected end of input encountered.",
			state.GetCurrentToken().m_lineNumber, state.GetCurrentToken().m_characterInLine);
	}
	state.Increment();

	const Token& token = state.GetCurrentToken();

	if (token.m_type == TokenType::OpenParen)
	{
		return ParseArgumentListHelper(state, true, TokenType::Comma, TokenType::CloseParen);
	}
	else if (token.m_type == TokenType::NewLine)
	{
		if ((!state.HasMoreTokens()) || state.PeekNextToken().m_type != TokenType::Indent)
		{
			return ParseArgumentListResult::Make<SyntaxError>("Multi line argument lists must be indented.",
				token.m_lineNumber, token.m_characterInLine);
		}
		state.Increment();
		return ParseArgumentListHelper(state, false, TokenType::NewLine, TokenType::Dedent);
	}
	
	return ParseArgumentListResult::Make<SyntaxError>(
		"Argument lists must be begin with an open parenthesis or an indented new line.",
		token.m_lineNumber, token.m_characterInLine);
}

ParseArgumentListResult ParseSingleLineArgumentList(ParsingState& state)
{
	if (!state.HasMoreTokens())
	{
		return ParseArgumentListResult::Make<SyntaxError>("Unexpected end of input encountered.",
			state.GetCurrentToken().m_lineNumber, state.GetCurrentToken().m_characterInLine);
	}
	state.Increment();

	const Token& token = state.GetCurrentToken();

	if (token.m_type != TokenType::OpenParen)
	{
		return ParseArgumentListResult::Make<SyntaxError>(
			"Argument lists within single-line expressions must begin with an open parenthesis.",
			token.m_lineNumber, token.m_characterInLine);
	}

	return ParseArgumentListHelper(state, true, TokenType::Comma, TokenType::CloseParen);
}

ParseExpressionResult ParseNodeExpression(const Token& nodeNameToken, ParsingState& state)
{
	std::string nodeName{ nodeNameToken.m_charsBegin, nodeNameToken.m_charsEnd };

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

ParseExpressionResult ParseSingleLineNodeExpression(const Token& nodeNameToken, ParsingState& state)
{
	std::string nodeName{ nodeNameToken.m_charsBegin, nodeNameToken.m_charsEnd };

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
	if (!state.HasMoreTokens())
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected end of input encountered.",
			state.GetCurrentToken().m_lineNumber, state.GetCurrentToken().m_characterInLine);
	}
	state.Increment();

	const Token& token = state.GetCurrentToken();
	
	switch (token.m_type)
	{
	case TokenType::Indent:
	case TokenType::Dedent:
	case TokenType::NewLine:
	{
		return ParseExpressionResult::Make<SyntaxError>(
			"Unexpected new line encountered; expected expression to be on a single line.",
			token.m_lineNumber, token.m_characterInLine);
	}
	case TokenType::OpenParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '(' encountered.",
			token.m_lineNumber, token.m_characterInLine);
	}
	case TokenType::CloseParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ')' encountered.",
			token.m_lineNumber, token.m_characterInLine);
	}
	case TokenType::OpenCurly:
	{
		return ParseInfixExpression(state);
	}
	case TokenType::CloseCurly:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '}' encountered.",
			token.m_lineNumber, token.m_characterInLine);
	}
	case TokenType::Comma:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ',' encountered.",
			token.m_lineNumber, token.m_characterInLine);
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
		const KeywordType keywordType = ConvertToKeyword(token.m_charsBegin, token.m_charsEnd);
		if (keywordType != KeywordType::Invalid)
		{
			return MakeLiteralExpressionFromKeywordType(state, keywordType);
		}
		else if (IsNodeName(token.m_charsBegin, token.m_charsEnd))
		{
			return ParseSingleLineNodeExpression(token, state);
		}
		else if (IsFunctionName(token.m_charsBegin, token.m_charsEnd))
		{
			std::string name{ token.m_charsBegin, token.m_charsEnd };
			
			// If the next token is an open parenthesis, this is a function call expression.
			// Otherwise, this is a tree identifier.
			if (!state.HasMoreTokens())
			{
				return ParseExpressionResult::Make<SyntaxError>("Unexpected end of input encountered.",
					token.m_lineNumber, token.m_characterInLine);
			}

			const Token& peekToken = state.PeekNextToken();

			if (peekToken.m_type == TokenType::OpenParen)
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
			else if (!IsTreeName(token.m_charsBegin, token.m_charsEnd))
			{
				return ParseExpressionResult::Make<SyntaxError>(
					"Unexpected function name encountered; expected a tree name.",
					token.m_lineNumber, token.m_characterInLine);
			}
			else
			{
				auto result = ParseExpressionResult::Make<Expression>();
				Expression& expression = result.Get<Expression>();

				expression.m_variant = decltype(Expression::m_variant)::Make<IdentifierExpression>(std::move(name));

				return result;
			}
		}
		
		Dev::FatalAssert(!IsTreeName(token.m_charsBegin, token.m_charsEnd),
			"All tree names should be handled by the function name condition.");

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
ParseExpressionResult ParseInfixExpression(ParsingState& state)
{
	const ParseExpressionResult leftExpressionResult = ParseSingleLineExpression(state);
	if (!leftExpressionResult.Is<Expression>())
	{
		return leftExpressionResult;
	}

	if (!state.HasMoreTokens())
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected end of input encountered.",
			state.GetCurrentToken().m_lineNumber, state.GetCurrentToken().m_characterInLine);
	}
	state.Increment();

	const Token& functionNameToken = state.GetCurrentToken();

	if (functionNameToken.m_type != TokenType::Text)
	{
		return ParseExpressionResult::Make<SyntaxError>(
			"Unexpected non-text token encountered; expected a function name.",
			functionNameToken.m_lineNumber, functionNameToken.m_characterInLine);
	}
	if (!IsFunctionName(functionNameToken.m_charsBegin, functionNameToken.m_charsEnd))
	{
		std::string message = "Expected a function name; encountered \"";
		message += std::string(functionNameToken.m_charsBegin, functionNameToken.m_charsEnd);
		message += "\".";

		return ParseExpressionResult::Make<SyntaxError>(std::move(message),
			functionNameToken.m_lineNumber, functionNameToken.m_characterInLine);
	}

	std::string functionName{ functionNameToken.m_charsBegin, functionNameToken.m_charsEnd };

	const ParseExpressionResult rightExpressionResult = ParseSingleLineExpression(state);
	if (!rightExpressionResult.Is<Expression>())
	{
		return rightExpressionResult;
	}

	if (!state.HasMoreTokens())
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected end of input encountered.",
			state.GetCurrentToken().m_lineNumber, state.GetCurrentToken().m_characterInLine);
	}
	state.Increment();

	const Token& closingCurlyToken = state.GetCurrentToken();
	if (closingCurlyToken.m_type != TokenType::CloseCurly)
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected non-'}' encountered.",
			closingCurlyToken.m_lineNumber, closingCurlyToken.m_characterInLine);
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
	if (!state.HasMoreTokens())
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected end of input encountered.",
			state.GetCurrentToken().m_lineNumber, state.GetCurrentToken().m_characterInLine);
	}
	state.Increment();

	const Token& token = state.GetCurrentToken();
	
	switch (token.m_type)
	{
	case TokenType::Indent:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected indent encountered.",
			token.m_lineNumber, token.m_characterInLine);
	}
	case TokenType::Dedent:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected dedent encountered.",
			token.m_lineNumber, token.m_characterInLine);
	}
	case TokenType::NewLine:
	{
		return ParseExpression(state);
	}
	case TokenType::OpenParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '(' encountered.",
			token.m_lineNumber, token.m_characterInLine);
	}
	case TokenType::CloseParen:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ')' encountered.",
			token.m_lineNumber, token.m_characterInLine);
	}
	case TokenType::OpenCurly:
	{
		return ParseInfixExpression(state);
	}
	case TokenType::CloseCurly:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected '}' encountered.",
			token.m_lineNumber, token.m_characterInLine);
	}
	case TokenType::Comma:
	{
		return ParseExpressionResult::Make<SyntaxError>("Unexpected ',' encountered.",
			token.m_lineNumber, token.m_characterInLine);
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
		const KeywordType keywordType = ConvertToKeyword(token.m_charsBegin, token.m_charsEnd);
		if (keywordType != KeywordType::Invalid)
		{
			return MakeLiteralExpressionFromKeywordType(state, keywordType);
		}
		else if (IsNodeName(token.m_charsBegin, token.m_charsEnd))
		{
			return ParseNodeExpression(token, state);
		}
		else if (IsFunctionName(token.m_charsBegin, token.m_charsEnd))
		{
			std::string name{ token.m_charsBegin, token.m_charsEnd };

			// If the next token is an open parenthesis or an indenting newline, this is a function call expression.
			// Otherwise, this is a tree identifier.
			if (!state.HasMoreTokens())
			{
				return ParseExpressionResult::Make<SyntaxError>("Unexpected end of input encountered.",
					state.GetCurrentToken().m_lineNumber, state.GetCurrentToken().m_characterInLine);
			}
			const Token& peekToken = state.PeekNextToken();

			if (peekToken.m_type == TokenType::OpenParen
				|| peekToken.m_type == TokenType::NewLine)
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
			else if (!IsTreeName(token.m_charsBegin, token.m_charsEnd))
			{
				return ParseExpressionResult::Make<SyntaxError>(
					"Unexpected function name encountered; expected a tree name.",
					token.m_lineNumber, token.m_characterInLine);
			}
			else
			{
				auto result = ParseExpressionResult::Make<Expression>();
				Expression& expression = result.Get<Expression>();

				expression.m_variant = decltype(Expression::m_variant)::Make<IdentifierExpression>(std::move(name));

				return result;
			}
		}

		Dev::FatalAssert(!IsTreeName(token.m_charsBegin, token.m_charsEnd),
			"All tree names should be handled by the function name condition.");

		return MakeNumericLiteralExpression(token);
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

	TokenizingState tokenizingState;
	tokenizingState.i = input;
	tokenizingState.currentIndent = 0;
	tokenizingState.lineNumber = 1;
	tokenizingState.characterInLine = 0;

	// Tokenize the input.
	Collection::Vector<Token> tokens;
	while (ParseNextTokens(tokenizingState, tokens)) {}

	// Transform the tokens into a ParsedTree.
	ParsingState parsingState{ tokens };

	while (parsingState.HasMoreTokens())
	{
		// Parse the "tree" keyword, skipping blank lines between trees.
		parsingState.Increment();
		if (parsingState.GetCurrentToken().m_type == TokenType::NewLine)
		{
			continue;
		}

		if (parsingState.GetCurrentToken().m_type != TokenType::Text)
		{
			return ParseResult::Make<SyntaxError>("Unexpected non-text token encountered: expected \"tree\".",
				parsingState.GetCurrentToken().m_lineNumber, parsingState.GetCurrentToken().m_characterInLine);
		}

		const std::ptrdiff_t tokenLength =
			parsingState.GetCurrentToken().m_charsEnd - parsingState.GetCurrentToken().m_charsBegin;

		if (tokenLength != 4 || strncmp(parsingState.GetCurrentToken().m_charsBegin, "tree", 4) != 0)
		{
			std::string message = "Unexpected token encountered: expected \"tree\", got \"";
			message += std::string(
				parsingState.GetCurrentToken().m_charsBegin, parsingState.GetCurrentToken().m_charsEnd);
			message += "\".";
			return ParseResult::Make<SyntaxError>(std::move(message), parsingState.GetCurrentToken().m_lineNumber,
				parsingState.GetCurrentToken().m_characterInLine);
		}

		// Parse the tree's name.
		if (!parsingState.HasMoreTokens())
		{
			return ParseResult::Make<SyntaxError>("Unexpected end of input encountered.",
				parsingState.GetCurrentToken().m_lineNumber, parsingState.GetCurrentToken().m_characterInLine);
		}
		parsingState.Increment();

		if (parsingState.GetCurrentToken().m_type != TokenType::Text)
		{
			return ParseResult::Make<SyntaxError>("Unexpected non-text token encountered: expected a tree name.",
				parsingState.GetCurrentToken().m_lineNumber, parsingState.GetCurrentToken().m_characterInLine);
		}
		if (!IsTreeName(parsingState.GetCurrentToken().m_charsBegin, parsingState.GetCurrentToken().m_charsEnd))
		{
			return ParseResult::Make<SyntaxError>(
				"Tree names must begin with an uppercase letter and consist only of alphanumeric characters.",
				parsingState.GetCurrentToken().m_lineNumber, parsingState.GetCurrentToken().m_characterInLine);
		}

		ParsedTree& parsedTree = outTrees.Emplace();
		parsedTree.m_treeName = std::string(
			parsingState.GetCurrentToken().m_charsBegin, parsingState.GetCurrentToken().m_charsEnd);

		// Ensure there is a newline after the tree's name.
		if (!parsingState.HasMoreTokens())
		{
			return ParseResult::Make<SyntaxError>("Unexpected end of input encountered.",
				parsingState.GetCurrentToken().m_lineNumber, parsingState.GetCurrentToken().m_characterInLine);
		}
		parsingState.Increment();

		if (parsingState.GetCurrentToken().m_type != TokenType::NewLine)
		{
			return ParseResult::Make<SyntaxError>(
				"A tree's root node must be on the line following the tree's name declaration.",
				parsingState.GetCurrentToken().m_lineNumber, parsingState.GetCurrentToken().m_characterInLine);
		}

		// Parse the tree's root node.
		ParseExpressionResult rootExpressionResult = ParseExpression(parsingState);
		if (!rootExpressionResult.IsAny())
		{
			// This only happens due to internal parser errors and is not part of normal control flow.
			return ParseResult();
		}

		SyntaxError rootSyntaxError{ "", parsingState.GetCurrentToken().m_lineNumber,
			parsingState.GetCurrentToken().m_characterInLine };

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
	}

	return ParseResult::Make<Collection::Vector<ParsedTree>>(std::move(outTrees));
}
}
