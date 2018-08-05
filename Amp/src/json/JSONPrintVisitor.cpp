#include <json/JSONPrintVisitor.h>

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONString& value)
{
	m_output << '"' << value.m_string << '"';
	return JSON::VisitorFlow::Visit;
}

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONNumber& value)
{
	m_output << value.m_number;
	return JSON::VisitorFlow::Visit;
}

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONBoolean& value)
{
	m_output << (value.m_boolean ? "true" : "false");
	return JSON::VisitorFlow::Visit;
}

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONNull& value)
{
	m_output  << "null";
	return JSON::VisitorFlow::Visit;
}

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONArray& value)
{
	m_output << '[' << std::endl;
	m_prefix.push_back('\t');

	for (const auto& element : value)
	{
		m_output << m_prefix;
		element->Accept(this);
		m_output << ',' << std::endl;
	}

	m_prefix.pop_back();
	m_output << m_prefix << ']';
	return JSON::VisitorFlow::Skip;
}

JSON::VisitorFlow JSON::PrintVisitor::Visit(const JSONObject& value)
{
	m_output << '{' << std::endl;
	m_prefix.push_back('\t');

	for (const auto& entry : value)
	{
		m_output << m_prefix << '"' << entry.first << '"' << " : ";
		entry.second->Accept(this);
		m_output << ',' << std::endl;
	}

	m_prefix.pop_back();
	m_output << m_prefix << '}';
	return JSON::VisitorFlow::Skip;
}
