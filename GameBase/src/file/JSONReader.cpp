#include <file/JSONReader.h>

#include <dev/Dev.h>
#include <file/FullFileReader.h>
#include <json/JSONTypes.h>

#include <cctype>
#include <sstream>

using namespace JSON;

namespace JSONReader
{
	bool TryReadJSONString(const char*& inOutInput, JSONString& out);
	bool TryReadJSONNumber(const char*& inOutInput, JSONNumber& out);
	bool TryReadJSONBoolean(const char*& inOutInput, JSONBoolean& out);
	bool TryReadJSONNull(const char*& inOutInput, JSONNull& out);
	bool TryReadJSONArray(const char*& inOutInput, JSONArray& out);
	bool TryReadJSONObject(const char*& inOutInput, JSONObject& out);

	Mem::UniquePtr<JSONValue> ReadJSONValue(const char*& inOutInput);
}

namespace
{
bool IsDigit(const char c)
{
	return (c >= '0' && c <= '9');
}

bool IsValueDelimeter(const char c)
{
	return (c == ',' || c == ':' || std::isspace(c));
}

bool TryIdentifyNextValue(const char firstCharacter, ValueType& outType)
{
	switch (firstCharacter)
	{
	case '"': { outType = ValueType::String; return true; }
	case '-': { outType = ValueType::Number; return true; }
	case 't': { outType = ValueType::Boolean; return true; }
	case 'f': { outType = ValueType::Boolean; return true; }
	case 'n': { outType = ValueType::Null; return true; }
	case '[': { outType = ValueType::Array; return true; }
	case '{': { outType = ValueType::Object; return true; }
	}

	if (IsDigit(firstCharacter))
	{
		outType = ValueType::Number;
		return true;
	}
	return false;
}

void ScanWhitespace(const char*& inOutInput)
{
	while (std::isspace(inOutInput[0]))
	{
		++inOutInput;
	}
}

bool TryToEscape(const char c, char& out)
{
	switch (c)
	{
	case '"': { out = '"'; return true; }
	case '\\': { out = '\\'; return true; }
	case '/': { out = '/'; return true; } // TODO is this one correct?
	case 'b': { out = '\b'; return true; }
	case 'f': { out = '\f'; return true; }
	case 'n': { out = '\n'; return true; }
	case 'r': { out = '\r'; return true; }
	case 't': { out = '\t'; return true; }
	default: { return false; }
	}
}
}

bool JSONReader::TryReadJSONString(const char*& inOutInput, JSONString& out)
{
	Dev::FatalAssert(inOutInput[0] == '"', "Strings must always start with a double quote.");

	// Advance past the opening quote.
	++inOutInput;

	// Take in characters and handle escape characters along the way.
	std::vector<char> buffer;
	while (inOutInput[0] != '\0')
	{
		// If we arrive at a double quote, the string is complete.
		if (inOutInput[0] == '"')
		{
			buffer.push_back('\0');
			out.m_string = buffer.data();
			out.m_hash = Util::CalcHash(out.m_string);
			++inOutInput;
			return true;
		}

		// If we arrive at anything other than a backslash, it's just part of the string.
		// If we arrive at a backslash, we need to verify that the escape character is correct.
		if (inOutInput[0] != '\\')
		{
			buffer.push_back(inOutInput[0]);
			++inOutInput;
		}
		else
		{
			// Advance past the backslash and try to escape the next character.
			// TODO Support the uXXX escape codes.
			++inOutInput;
			char escapedCharacter;
			if (!TryToEscape(inOutInput[0], escapedCharacter))
			{
				return false;
			}
			buffer.push_back(escapedCharacter);
			++inOutInput;
		}
	}

	// If we exhaust the input before encountering a closing double quote, we failed to read the string.
	return false;
}

bool JSONReader::TryReadJSONNumber(const char*& inOutInput, JSONNumber& out)
{
	const bool isNegative = (inOutInput[0] == '-');
	if (isNegative)
	{
		++inOutInput;
	}

	// Parse the integer part of the number.
	while (IsDigit(inOutInput[0]))
	{
		out.m_number *= 10.0;
		out.m_number += static_cast<double>(inOutInput[0] - '0');
		++inOutInput;
	}

	// Parse the fractional part of the number, if it exists.
	// Otherwise, verify that the number ended.
	if (inOutInput[0] == '.')
	{
		++inOutInput;

		uint32_t fractionalPart = 0;
		size_t numFractionalDigits = 0;
		for (; IsDigit(inOutInput[0]); ++numFractionalDigits)
		{
			fractionalPart *= 10;
			fractionalPart += static_cast<uint32_t>(inOutInput[0] - '0');
			++inOutInput;
		}
		double fraction = static_cast<double>(fractionalPart);
		for (size_t i = 0; i < numFractionalDigits; ++i)
		{
			fraction *= 0.1;
		}
		out.m_number += fraction;
	}
	else if (!IsValueDelimeter(inOutInput[0]))
	{
		return false;
	}

	// Parse the exponent part of the number, if it exists.
	if (inOutInput[0] == 'e' || inOutInput[0] == 'E')
	{
		++inOutInput;
		const bool negativeExponent = (inOutInput[0] == '-');
		if (inOutInput[0] == '+' || inOutInput[0] == '-')
		{
			++inOutInput;
		}

		size_t exponent = 0;
		while (IsDigit(inOutInput[0]))
		{
			exponent *= 10;
			exponent += static_cast<size_t>(inOutInput[0] - '0');
			++inOutInput;
		}

		uint32_t multiplier = 1;
		for (size_t i = 0; i < exponent; ++i)
		{
			multiplier *= 10;
		}

		if (negativeExponent)
		{
			out.m_number /= multiplier;
		}
		else
		{
			out.m_number *= multiplier;
		}
	}

	// Apply the sign and verify that the number ended.
	if (isNegative)
	{
		out.m_number *= -1.0;
	}
	return IsValueDelimeter(inOutInput[0]);
}

bool JSONReader::TryReadJSONBoolean(const char*& inOutInput, JSONBoolean& out)
{
	if (strncmp(inOutInput, "true", 4) == 0)
	{
		out.m_boolean = true;
		inOutInput += 4;
		return true;
	}
	if (strncmp(inOutInput, "false", 5) == 0)
	{
		out.m_boolean = false;
		inOutInput += 5;
		return true;
	}
	return false;
}

bool JSONReader::TryReadJSONNull(const char*& inOutInput, JSONNull& out)
{
	if (strncmp(inOutInput, "null", 4) == 0)
	{
		inOutInput += 4;
		return true;
	}
	return false;
}

bool JSONReader::TryReadJSONArray(const char*& inOutInput, JSONArray& out)
{
	Dev::FatalAssert(inOutInput[0] == '[', "Arrays must always start with a square bracket.");
	
	// Advance past the opening bracket and any opening whitespace.
	++inOutInput;
	ScanWhitespace(inOutInput);

	// Read values until the array ends or we exhaust the input.
	while (inOutInput[0] != '\0')
	{
		// If we arrive at a closing bracket, the array is done.
		if (inOutInput[0] == ']')
		{
			++inOutInput;
			return true;
		}

		// Read the value. If reading the value failed, reading the array failed.
		Mem::UniquePtr<JSONValue> value = ReadJSONValue(inOutInput);
		if (value == nullptr)
		{
			return false;
		}
		Dev::FatalAssert(IsValueDelimeter(inOutInput[0]),
			"If this isn't at a value delimeter at this point, something is seriously wrong.");

		// Inform the value of its key and store it in the output array.
		value->m_key = JSONKey(out.Size());
		out.Add(std::move(value));

		// Scan past whitespace. If a comma is encountered, skip it and scan whitespace again.
		ScanWhitespace(inOutInput);
		if (inOutInput[0] == ',')
		{
			++inOutInput;
			ScanWhitespace(inOutInput);
		}
	}

	// If the input ended before encountering a closing bracket, we failed to read the array.
	return false;
}

bool JSONReader::TryReadJSONObject(const char*& inOutInput, JSONObject& out)
{
	Dev::FatalAssert(inOutInput[0] == '{', "Objects must always start with a curly bracket.");
	
	// Advance past the opening bracket and any opening whitespace.
	++inOutInput;
	ScanWhitespace(inOutInput);

	// Read string/value pairs from the object until the object or input ends.
	while (inOutInput[0] != '\0')
	{
		// If we arrive at a closing bracket, the object is done.
		if (inOutInput[0] == '}')
		{
			++inOutInput;
			return true;
		}

		// A string key must be encountered here for the object to be valid.
		JSONString key;
		if (!TryReadJSONString(inOutInput, key))
		{
			return false;
		}

		// Scan past whitespace until a colon is encountered, then scan past whitespace again.
		// If a colon is not encountered, the object is not valid.
		ScanWhitespace(inOutInput);
		if (inOutInput[0] != ':')
		{
			return false;
		}
		++inOutInput;
		ScanWhitespace(inOutInput);

		// A value must be encountered here for the object to be valid.
		Mem::UniquePtr<JSONValue> value = ReadJSONValue(inOutInput);
		if (value == nullptr)
		{
			return false;
		}

		// Store the key/value pair in the output object.
		JSONObject::Entry& entry = out.Emplace(key.m_string, std::move(value));

		// Because string keys are weak pointers, assign the key in the value now
		// that it has been copied into the output object.
		entry.second->m_key = JSONKey(entry.first.data());

		// Scan past whitespace. If a comma is encountered, skip it and scan whitespace again.
		ScanWhitespace(inOutInput);
		if (inOutInput[0] == ',')
		{
			++inOutInput;
			ScanWhitespace(inOutInput);
		}
	}

	// If the input ended before encountering a closing bracket, we failed to read the object.
	return false;
}

Mem::UniquePtr<JSONValue> JSONReader::ReadJSONValue(const char*& inOutInput)
{
	ValueType valueType;
	if (!TryIdentifyNextValue(inOutInput[0], valueType))
	{
		return nullptr;
	}

	switch (valueType)
	{
	case ValueType::String:
	{
		auto value = Mem::MakeUnique<JSONString>();
		return (TryReadJSONString(inOutInput, *value)) ? std::move(value) : nullptr;
	}
	case ValueType::Number:
	{
		auto value = Mem::MakeUnique<JSONNumber>();
		return (TryReadJSONNumber(inOutInput, *value)) ? std::move(value) : nullptr;
	}
	case ValueType::Boolean:
	{
		auto value = Mem::MakeUnique<JSONBoolean>();
		return (TryReadJSONBoolean(inOutInput, *value)) ? std::move(value) : nullptr;
	}
	case ValueType::Null:
	{
		auto value = Mem::MakeUnique<JSONNull>();
		return (TryReadJSONNull(inOutInput, *value)) ? std::move(value) : nullptr;
	}
	case ValueType::Array:
	{
		auto value = Mem::MakeUnique<JSONArray>();
		return (TryReadJSONArray(inOutInput, *value)) ? std::move(value) : nullptr;
	}
	case ValueType::Object:
	{
		auto value = Mem::MakeUnique<JSONObject>();
		return (TryReadJSONObject(inOutInput, *value)) ? std::move(value) : nullptr;
	}
	}
	return nullptr;
}

Mem::UniquePtr<JSONValue> File::ReadJSONFile(const File::Path& path)
{
	const std::string fullText = ReadFullTextFile(path);

	const char* input = fullText.data();
	return JSONReader::ReadJSONValue(input);
}
