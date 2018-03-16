#include <json/JSONPrintVisitor.h>

#include <iostream>

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONString& value)
{
	std::cout << '"' << value.m_string << '"';
	return JSON::VisitorFlow::Visit;
}

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONNumber& value)
{
	std::cout << value.m_number;
	return JSON::VisitorFlow::Visit;
}

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONBoolean& value)
{
	std::cout << (value.m_boolean ? "true" : "false");
	return JSON::VisitorFlow::Visit;
}

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONNull& value)
{
	std::cout << "null";
	return JSON::VisitorFlow::Visit;
}

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONArray& value)
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
	return JSON::VisitorFlow::Skip;
}

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONObject& value)
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
	return JSON::VisitorFlow::Skip;
}
