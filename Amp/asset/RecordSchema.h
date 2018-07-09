#pragma once

#include <asset/RecordSchemaField.h>
#include <collection/Vector.h>
#include <collection/VectorMap.h>

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

	// A mapping of types to includes this schema depends on.
	Collection::VectorMap<std::string, std::string> m_importedTypes{};

	// A list of fields sorted by field ID.
	Collection::Vector<RecordSchemaField> m_fields{};

public:
	RecordSchema() = default;

	static RecordSchema MakeFromJSON(const JSON::JSONObject& jsonObject);

	uint32_t GetVersion() const { return m_version; }
	const std::string& GetName() const { return m_name; }

	const Collection::VectorMap<std::string, std::string>& GetImportedTypes() const { return m_importedTypes; }

	RecordSchemaField* FindField(uint16_t fieldID);
	const RecordSchemaField* FindField(uint16_t fieldID) const;

	bool Accept(RecordSchemaVisitor& visitor, uint16_t fieldID) const;

	bool CheckIsErrorFree() const;
};
}
