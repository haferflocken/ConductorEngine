#pragma once

#include <asset/RecordSchemaField.h>
#include <collection/Vector.h>

#include <string>

namespace JSON { class JSONObject; }

namespace Asset
{
class RecordSchemaVisitor;

/**
 * Defines the data layout of a type of record.
 */
class RecordSchema
{
	// The version number of this schema.
	uint32_t m_version{ 0 };

	// The name of this schema.
	std::string m_name;

	// A list of fields sorted by field ID.
	Collection::Vector<RecordSchemaField> m_fields{};

public:
	RecordSchema() = default;

	static RecordSchema MakeFromJSON(const JSON::JSONObject& jsonObject);

	uint32_t GetVersion() const { return m_version; }
	const std::string& GetName() const { return m_name; }

	RecordSchemaField* FindField(uint16_t fieldID);
	const RecordSchemaField* FindField(uint16_t fieldID) const;

	bool Accept(RecordSchemaVisitor& visitor, uint16_t fieldID) const;

	bool CheckIsErrorFree() const;
};
}
