#pragma once

#include <json/JSONTypes.h>

namespace JSON
{
class PrintVisitor : public Visitor
{
public:
	PrintVisitor()
		: m_prefix("")
	{}

	virtual VisitorFlow Visit(const JSONString& value) override;
	virtual VisitorFlow Visit(const JSONNumber& value) override;
	virtual VisitorFlow Visit(const JSONBoolean& value) override;
	virtual VisitorFlow Visit(const JSONNull& value) override;
	virtual VisitorFlow Visit(const JSONArray& value) override;
	virtual VisitorFlow Visit(const JSONObject& value) override;

private:
	std::string m_prefix;
};
}
