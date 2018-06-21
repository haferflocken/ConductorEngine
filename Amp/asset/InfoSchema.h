#pragma once

#include <asset/InfoSchemaField.h>
#include <collection/Vector.h>

namespace JSON { class JSONObject; }

namespace Asset
{
/**
 * Defines the data layout of a class of text data files.
 */
class InfoSchema
{
	// The version number of this schema.
	uint32_t m_version{ 0 };

	// A list of fields sorted by field ID.
	Collection::Vector<InfoSchemaField> m_fields{};

public:
	InfoSchema() = default;

	static InfoSchema MakeFromJSON(const JSON::JSONObject& jsonObject);

	uint32_t GetVersion() const { return m_version; }
	InfoSchemaField* FindField(uint16_t fieldID);
	const InfoSchemaField* FindField(uint16_t fieldID) const;
};
}
