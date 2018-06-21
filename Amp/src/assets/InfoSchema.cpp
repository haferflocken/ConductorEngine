#include <asset/InfoSchema.h>

#include <json/JSONTypes.h>

namespace Asset
{
namespace Internal_InfoSchema
{
const Util::StringHash k_versionHash = Util::CalcHash("version");
const Util::StringHash k_fieldsHash = Util::CalcHash("fields");

const Util::StringHash k_fieldTypeHash = Util::CalcHash("fieldType");
const Util::StringHash k_fieldIDHash = Util::CalcHash("fieldID");

class MakeFieldVisitor : public JSON::Visitor
{
	Collection::Vector<InfoSchemaField>& m_destination;

public:
	MakeFieldVisitor(Collection::Vector<InfoSchemaField>& destination)
		: m_destination(destination)
	{}

	virtual ~MakeFieldVisitor() {}

	JSON::VisitorFlow Visit(const JSON::JSONString& value) override { return JSON::VisitorFlow::Stop; }
	JSON::VisitorFlow Visit(const JSON::JSONNumber& value) override { return JSON::VisitorFlow::Stop; }
	JSON::VisitorFlow Visit(const JSON::JSONBoolean& value) override { return JSON::VisitorFlow::Stop; }
	JSON::VisitorFlow Visit(const JSON::JSONNull& value) override { return JSON::VisitorFlow::Stop; }
	JSON::VisitorFlow Visit(const JSON::JSONArray& value) override { return JSON::VisitorFlow::Stop; }

	JSON::VisitorFlow Visit(const JSON::JSONObject& value) override
	{
		const JSON::JSONString* const fieldType = value.FindString(k_fieldTypeHash);
		if (fieldType == nullptr)
		{
			return JSON::VisitorFlow::Stop;
		}
		
		const JSON::JSONNumber* const fieldID = value.FindNumber(k_fieldIDHash);
		if (fieldID == nullptr)
		{
			return JSON::VisitorFlow::Stop;
		}

		// TODO(assets) visit
		return JSON::VisitorFlow::Skip;
	}
};
}

InfoSchema InfoSchema::MakeFromJSON(const JSON::JSONObject& jsonObject)
{
	using namespace Internal_InfoSchema;

	InfoSchema outSchema;

	const JSON::JSONNumber* const version = jsonObject.FindNumber(k_versionHash);
	if (version == nullptr)
	{
		return outSchema;
	}
	const JSON::JSONArray* const fields = jsonObject.FindArray(k_fieldsHash);
	if (fields == nullptr)
	{
		return outSchema;
	}

	outSchema.m_version = static_cast<uint32_t>(version->m_number);
	
	MakeFieldVisitor visitor{ outSchema.m_fields };
	for (const auto& field : *fields)
	{
		field->Accept(&visitor);
	}

	return outSchema;
}
}
