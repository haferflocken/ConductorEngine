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

const Util::StringHash k_fieldDefaultValueHash = Util::CalcHash("defaultValue");
const Util::StringHash k_fieldMinValueHash = Util::CalcHash("minValue");
const Util::StringHash k_fieldMaxValueHash = Util::CalcHash("maxValue");

const Util::StringHash k_fieldAcceptedTypesHash = Util::CalcHash("acceptedTypes");
const Util::StringHash k_fieldMemberIDsHash = Util::CalcHash("memberIDs");

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
			Dev::LogWarning("Skipped a field which was missing an ID.");
			continue;
		}
		const uint16_t fieldID = static_cast<uint16_t>(candidateID->m_number);

		const JSON::JSONString* const candidateType = field.FindString(k_fieldTypeHash);
		if (candidateType == nullptr)
		{
			Dev::LogWarning("Skipped field [%u] with a missing type.", fieldID);
			continue;
		}

		const InfoSchemaFieldType fieldType = ParseType(candidateType->m_string.c_str());
		if (fieldType == InfoSchemaFieldType::Invalid)
		{
			Dev::LogWarning("Skipped field [%u] with unknown type [%s].", fieldID, candidateType->m_string.c_str());
			continue;
		}

		switch (fieldType)
		{
		case InfoSchemaFieldType::Boolean:
		{
			InfoSchemaField booleanField = InfoSchemaField::MakeBooleanField(fieldID);

			const JSON::JSONBoolean* const candidateDefault = field.FindBoolean(k_fieldDefaultValueHash);
			if (candidateDefault != nullptr)
			{
				booleanField.m_booleanData.m_defaultValue = candidateDefault->m_boolean;
			}

			outSchema.m_fields.Add(std::move(booleanField));
			break;
		}
		case InfoSchemaFieldType::Float:
		{
			InfoSchemaField floatField = InfoSchemaField::MakeFloatField(fieldID);

			const JSON::JSONNumber* const candidateDefault = field.FindNumber(k_fieldDefaultValueHash);
			const JSON::JSONNumber* const candidateMin = field.FindNumber(k_fieldMinValueHash);
			const JSON::JSONNumber* const candidateMax = field.FindNumber(k_fieldMaxValueHash);
			if (candidateDefault != nullptr)
			{
				floatField.m_floatData.m_defaultValue = static_cast<float>(candidateDefault->m_number);
			}
			if (candidateMin != nullptr)
			{
				floatField.m_floatData.m_minimumValue = static_cast<float>(candidateMin->m_number);
			}
			if (candidateMax != nullptr)
			{
				floatField.m_floatData.m_maximumValue = static_cast<float>(candidateMax->m_number);
			}

			outSchema.m_fields.Add(std::move(floatField));
			break;
		}
		case InfoSchemaFieldType::Integer:
		{
			InfoSchemaField integerField = InfoSchemaField::MakeIntegerField(fieldID);

			const JSON::JSONNumber* const candidateDefault = field.FindNumber(k_fieldDefaultValueHash);
			const JSON::JSONNumber* const candidateMin = field.FindNumber(k_fieldMinValueHash);
			const JSON::JSONNumber* const candidateMax = field.FindNumber(k_fieldMaxValueHash);
			if (candidateDefault != nullptr)
			{
				integerField.m_integerData.m_defaultValue = static_cast<int32_t>(candidateDefault->m_number);
			}
			if (candidateMin != nullptr)
			{
				integerField.m_integerData.m_minimumValue = static_cast<int32_t>(candidateMin->m_number);
			}
			if (candidateMax != nullptr)
			{
				integerField.m_integerData.m_maximumValue = static_cast<int32_t>(candidateMax->m_number);
			}

			outSchema.m_fields.Add(std::move(integerField));
			break;
		}
		case InfoSchemaFieldType::InstanceReference:
		{
			InfoSchemaField instanceReferenceField = InfoSchemaField::MakeInstanceReferenceField(fieldID);

			const JSON::JSONArray* const candidateAcceptedTypes = field.FindArray(k_fieldAcceptedTypesHash);
			if (candidateAcceptedTypes != nullptr)
			{
				for (const auto& candidateAcceptedType : *candidateAcceptedTypes)
				{
					if (candidateAcceptedType->GetType() != JSON::ValueType::String)
					{
						Dev::LogWarning("Skipped non-string accepted type in field [%u].", fieldID);
						continue;
					}
					const JSON::JSONString& acceptedType =
						*dynamic_cast<const JSON::JSONString*>(candidateAcceptedType.Get());
					instanceReferenceField.m_instanceReferenceData.m_acceptedTypes.Add(acceptedType.m_string);
				}
			}

			outSchema.m_fields.Add(std::move(instanceReferenceField));
			break;
		}
		case InfoSchemaFieldType::Group:
		{
			InfoSchemaField groupField = InfoSchemaField::MakeGroupField(fieldID);

			const JSON::JSONArray* const candidateMemberFieldIDs = field.FindArray(k_fieldMemberIDsHash);
			if (candidateMemberFieldIDs != nullptr)
			{
				for (const auto& candidateMemberFieldID : *candidateMemberFieldIDs)
				{
					if (candidateMemberFieldID->GetType() != JSON::ValueType::Number)
					{
						Dev::LogWarning("Skipped non-number member field ID in field [%u].", fieldID);
						continue;
					}
					const JSON::JSONNumber& memberFieldID =
						*dynamic_cast<const JSON::JSONNumber*>(candidateMemberFieldID.Get());
					groupField.m_groupData.m_memberFieldIDs.Add(static_cast<uint16_t>(memberFieldID.m_number));
				}
			}

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
