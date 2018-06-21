#pragma once

#include <asset/RecordSchemaField.h>
#include <collection/Vector.h>

namespace JSON { class JSONObject; }

namespace Asset
{
/**
 * Defines the data layout of a type of record.
 */
class RecordSchema
{
	// The version number of this schema.
	uint32_t m_version{ 0 };

	// A list of fields sorted by field ID.
	Collection::Vector<RecordSchemaField> m_fields{};

public:
	RecordSchema() = default;

	static RecordSchema MakeFromJSON(const JSON::JSONObject& jsonObject);

	uint32_t GetVersion() const { return m_version; }
	RecordSchemaField* FindField(uint16_t fieldID);
	const RecordSchemaField* FindField(uint16_t fieldID) const;
};
}
