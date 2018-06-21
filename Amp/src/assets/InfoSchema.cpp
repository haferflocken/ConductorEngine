#include <asset/InfoSchema.h>

#include <json/JSONTypes.h>

#include <algorithm>

namespace Asset
{
namespace Internal_InfoSchema
{
const Util::StringHash k_versionHash = Util::CalcHash("version");
const Util::StringHash k_fieldsHash = Util::CalcHash("fields");

const Util::StringHash k_fieldTypeHash = Util::CalcHash("fieldType");
const Util::StringHash k_fieldIDHash = Util::CalcHash("fieldID");

InfoSchemaFieldType ParseType(const char* const typeString)
{
	if (strcmp("bool", typeString) == 0)
	{
		return InfoSchemaFieldType::Boolean;
	}
	if (strcmp("float", typeString) == 0)
	{
		return InfoSchemaFieldType::Float;
	}
	if (strcmp("int", typeString) == 0)
	{
		return InfoSchemaFieldType::Integer;
	}
	if (strcmp("instanceReference", typeString) == 0)
	{
		return InfoSchemaFieldType::InstanceReference;
	}
	if (strcmp("group", typeString) == 0)
	{
		return InfoSchemaFieldType::Group;
	}
	return InfoSchemaFieldType::Invalid;
}
}

InfoSchema InfoSchema::MakeFromJSON(const JSON::JSONObject& jsonObject)
{
	using namespace Internal_InfoSchema;

	InfoSchema outSchema;

	const JSON::JSONNumber* const version = jsonObject.FindNumber(k_versionHash);
	if (version == nullptr)
	{
		Dev::LogWarning("Failed to find a version number in the schema.");
		return outSchema;
	}
	const JSON::JSONArray* const fields = jsonObject.FindArray(k_fieldsHash);
	if (fields == nullptr)
	{
		Dev::LogWarning("Failed to find fields in the schema.");
		return outSchema;
	}

	outSchema.m_version = static_cast<uint32_t>(version->m_number);

	for (const auto& candidate : *fields)
	{
		if (candidate->GetType() != JSON::ValueType::Object)
		{
			Dev::LogWarning("Skipped a JSON value with type [%d] in the field array.",
				static_cast<int32_t>(candidate->GetType()));
			continue;
		}
		const auto& field = *dynamic_cast<const JSON::JSONObject*>(candidate.Get());

		const JSON::JSONNumber* const candidateID = field.FindNumber(k_fieldIDHash);
		if (candidateID == nullptr)
		{
			Dev::LogWarning("Skipped a field which was missing an ID in the field array.");
			continue;
		}
		const uint16_t fieldID = static_cast<uint16_t>(candidateID->m_number);

		const JSON::JSONString* const candidateType = field.FindString(k_fieldTypeHash);
		if (candidateType == nullptr)
		{
			Dev::LogWarning("Skipped field [%u] with a missing type in the field array.", fieldID);
			continue;
		}

		const InfoSchemaFieldType fieldType = ParseType(candidateType->m_string.c_str());
		if (fieldType == InfoSchemaFieldType::Invalid)
		{
			Dev::LogWarning("Skipped field [%u] with unknown type [%s] in the field array.", fieldID,
				candidateType->m_string.c_str());
			continue;
		}

		switch (fieldType)
		{
		case InfoSchemaFieldType::Boolean:
		{
			InfoSchemaField booleanField = InfoSchemaField::MakeBooleanField(fieldID);
			outSchema.m_fields.Add(std::move(booleanField));
			break;
		}
		case InfoSchemaFieldType::Float:
		{
			InfoSchemaField floatField = InfoSchemaField::MakeFloatField(fieldID);
			outSchema.m_fields.Add(std::move(floatField));
			break;
		}
		case InfoSchemaFieldType::Integer:
		{
			InfoSchemaField integerField = InfoSchemaField::MakeIntegerField(fieldID);
			outSchema.m_fields.Add(std::move(integerField));
			break;
		}
		case InfoSchemaFieldType::InstanceReference:
		{
			InfoSchemaField instanceReferenceField = InfoSchemaField::MakeInstanceReferenceField(fieldID);
			outSchema.m_fields.Add(std::move(instanceReferenceField));
			break;
		}
		case InfoSchemaFieldType::Group:
		{
			InfoSchemaField groupField = InfoSchemaField::MakeGroupField(fieldID);
			outSchema.m_fields.Add(std::move(groupField));
			break;
		}
		default:
		{
			Dev::FatalError("Encountered unhandled field type [%d].", static_cast<int32_t>(fieldType));
			break;
		}
		}
	}

	std::sort(outSchema.m_fields.begin(), outSchema.m_fields.end(),
		[](const InfoSchemaField& lhs, const InfoSchemaField& rhs) { return lhs.m_fieldID < rhs.m_fieldID; });

	return outSchema;
}

InfoSchemaField* InfoSchema::FindField(uint16_t fieldID)
{
	for (auto& field : m_fields)
	{
		if (field.m_fieldID == fieldID)
		{
			return &field;
		}
	}
	return nullptr;
}

const InfoSchemaField* InfoSchema::FindField(uint16_t fieldID) const
{
	for (const auto& field : m_fields)
	{
		if (field.m_fieldID == fieldID)
		{
			return &field;
		}
	}
	return nullptr;
}
}
