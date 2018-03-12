#include <json/JSONPrintVisitor.h>

#include <iostream>

using namespace JSON;

VisitorFlow PrintVisitor::Visit(const JSONString& value)
{
	std::cout << '"' << value.m_string << '"';
	return VisitorFlow::Visit;
}

VisitorFlow PrintVisitor::Visit(const JSONNumber& value)
{
	std::cout << value.m_number;
	return VisitorFlow::Visit;
}

VisitorFlow PrintVisitor::Visit(const JSONBoolean& value)
{
	std::cout << (value.m_boolean ? "true" : "false");
	return VisitorFlow::Visit;
}

VisitorFlow PrintVisitor::Visit(const JSONNull& value)
{
	std::cout << "null";
	return VisitorFlow::Visit;
}

VisitorFlow PrintVisitor::Visit(const JSONArray& value)
{
	std::cout << '[' << std::endl;
	m_prefix.push_back('\t');

	for (const auto& element : value)
	{
		std::cout << m_prefix;
		element->Accept(this);
		std::cout << ',' << std::endl;
	}

	m_prefix.pop_back();
	std::cout << m_prefix << ']';
	return VisitorFlow::Skip;
}

VisitorFlow PrintVisitor::Visit(const JSONObject& value)
{
	std::cout << '{' << std::endl;
	m_prefix.push_back('\t');

	for (const auto& entry : value)
	{
		std::cout << m_prefix << '"' << entry.first << '"' << " : ";
		entry.second->Accept(this);
		std::cout << ',' << std::endl;
	}

	m_prefix.pop_back();
	std::cout << m_prefix << '}';
	return VisitorFlow::Skip;
}
