#pragma once

#include <sstream>
#include <string>

namespace CodeGen
{
/**
 * A stream that builds a string with utilities for outputting C++.
 */
class CppStream
{
public:
	CppStream() = default;

	CppStream& operator<<(const char* text)
	{
		m_stream << text;
		return *this;
	}

	CppStream& operator<<(char character)
	{
		m_stream << character;
		return *this;
	}

	void NewLine()
	{
		m_stream << "\n" << m_indentation;
	}

	template <typename FnType>
	void Indent(FnType&& fn)
	{
		m_indentation.push_back('\t');
		fn();
		m_indentation.pop_back();
	}

	std::string CopyOut() const { return m_stream.str(); }

private:
	std::stringstream m_stream{};
	std::string m_indentation{};
};
}
