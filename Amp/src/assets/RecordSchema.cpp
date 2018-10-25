#include <asset/RecordSchema.h>

#include <asset/RecordSchemaVisitor.h>
#include <json/JSONTypes.h>

#include <algorithm>
#include <cstdlib>

namespace Asset
{
namespace Internal_RecordSchema
{
const Util::StringHash k_versionHash = Util::CalcHash("version");
const Util::StringHash k_nameHash = Util::CalcHash("name");
const Util::StringHash k_importedTypesHash = Util::CalcHash("importedTypes");
const Util::StringHash k_fieldsHash = Util::CalcHash("fields");

const Util::StringHash k_fieldNameHash = Util::CalcHash("name");
const Util::StringHash k_fieldDescriptionHash = Util::CalcHash("description");
const Util::StringHash k_fieldTypeHash = Util::CalcHash("type");

const Util::StringHash k_fieldDefaultValueHash = Util::CalcHash("defaultValue");
const Util::StringHash k_fieldMinValueHash = Util::CalcHash("minValue");
const Util::StringHash k_fieldMaxValueHash = Util::CalcHash("maxValue");

const Util::StringHash k_fieldImportedTypeHash = Util::CalcHash("importedType");
const Util::StringHash k_fieldAcceptedTypesHash = Util::CalcHash("acceptedTypes");
const Util::StringHash k_fieldMemberIDsHash = Util::CalcHash("memberIDs");
const Util::StringHash k_fieldElementIDHash = Util::CalcHash("elementID");

RecordSchemaFieldType ParseType(const char* const typeString)
{
	if (strcmp("bool", typeString) == 0)
	{
		return RecordSchemaFieldType::Boolean;
	}
	if (strcmp("float", typeString) == 0)
	{
		return RecordSchemaFieldType::Float;
	}
	if (strcmp("int", typeString) == 0)
	{
		return RecordSchemaFieldType::Integer;
	}
	if (strcmp("instanceReference", typeString) == 0)
	{
		return RecordSchemaFieldType::InstanceReference;
	}
	if (strcmp("importedType", typeString) == 0)
	{
		return RecordSchemaFieldType::ImportedType;
	}
	if (strcmp("group", typeString) == 0)
	{
		return RecordSchemaFieldType::Group;
	}
	if (strcmp("list", typeString) == 0)
	{
		return RecordSchemaFieldType::List;
	}
	return RecordSchemaFieldType::Invalid;
}
}

RecordSchema RecordSchema::MakeFromJSON(const JSON::JSONObject& jsonObject)
{
	using namespace Internal_RecordSchema;

	RecordSchema outSchema;

	const JSON::JSONNumber* const version = jsonObject.FindNumber(k_versionHash);
	if (version == nullptr)
	{
		AMP_LOG_WARNING("Failed to find a version number in the schema.");
		return outSchema;
	}
	const JSON::JSONString* const name = jsonObject.FindString(k_nameHash);
	if (name == nullptr || name->m_string.empty())
	{
		AMP_LOG_WARNING("Failed to find a name in the schema.");
		return outSchema;
	}
	const JSON::JSONObject* const fieldMap = jsonObject.FindObject(k_fieldsHash);
	if (fieldMap == nullptr)
	{
		AMP_LOG_WARNING("Failed to find fields in the schema.");
		return outSchema;
	}

	outSchema.m_version = static_cast<uint32_t>(version->m_number);
	outSchema.m_name = name->m_string;

	const JSON::JSONObject* const importedTypes = jsonObject.FindObject(k_importedTypesHash);
	if (importedTypes != nullptr)
	{
		for (const auto& entry : *importedTypes)
		{
			if (entry.second->GetType() != JSON::ValueType::String)
			{
				continue;
			}

			const char* const importedType =
				dynamic_cast<const JSON::JSONString*>(entry.second.Get())->m_string.c_str();
			outSchema.m_importedTypes[entry.first] = importedType;
		}
	}

	for (const auto& candidateEntry : *fieldMap)
	{
		// Validate the field input.
		const char* const candidateKey = candidateEntry.first.c_str();
		const JSON::JSONValue& candidate = *candidateEntry.second;

		if (candidate.GetType() != JSON::ValueType::Object)
		{
			AMP_LOG_WARNING("Skipped a JSON value with type [%d] in the field array.",
				static_cast<int32_t>(candidate.GetType()));
			continue;
		}
		const auto& field = *dynamic_cast<const JSON::JSONObject*>(&candidate);

		const uint16_t fieldID = static_cast<uint16_t>(strtol(candidateKey, nullptr, 10));

		const JSON::JSONString* const candidateName = field.FindString(k_fieldNameHash);
		if (candidateName == nullptr || candidateName->m_string.empty())
		{
			AMP_LOG_WARNING("Skipped field [%u] with a missing name.", fieldID);
			continue;
		}
		const char* const fieldName = candidateName->m_string.c_str();

		const JSON::JSONString* const candidateType = field.FindString(k_fieldTypeHash);
		if (candidateType == nullptr)
		{
			AMP_LOG_WARNING("Skipped field [%u] with a missing type.", fieldID);
			continue;
		}

		const RecordSchemaFieldType fieldType = ParseType(candidateType->m_string.c_str());
		if (fieldType == RecordSchemaFieldType::Invalid)
		{
			AMP_LOG_WARNING("Skipped field [%u] with unknown type [%s].", fieldID, candidateType->m_string.c_str());
			continue;
		}

		const JSON::JSONString* const candidateDescription = field.FindString(k_fieldDescriptionHash);
		const char* const fieldDescription =
			(candidateDescription != nullptr) ? candidateDescription->m_string.c_str() : "";

		// Create the field.
		switch (fieldType)
		{
		case RecordSchemaFieldType::Boolean:
		{
			RecordSchemaField booleanField = RecordSchemaField::MakeBooleanField(fieldID, fieldName, fieldDescription);

			const JSON::JSONBoolean* const candidateDefault = field.FindBoolean(k_fieldDefaultValueHash);
			if (candidateDefault != nullptr)
			{
				booleanField.m_booleanData.m_defaultValue = candidateDefault->m_boolean;
			}

			outSchema.m_fields.Add(std::move(booleanField));
			break;
		}
		case RecordSchemaFieldType::Float:
		{
			RecordSchemaField floatField = RecordSchemaField::MakeFloatField(fieldID, fieldName, fieldDescription);

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
		case RecordSchemaFieldType::Integer:
		{
			RecordSchemaField integerField = RecordSchemaField::MakeIntegerField(fieldID, fieldName, fieldDescription);

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
		case RecordSchemaFieldType::InstanceReference:
		{
			RecordSchemaField instanceReferenceField =
				RecordSchemaField::MakeInstanceReferenceField(fieldID, fieldName, fieldDescription);

			const JSON::JSONArray* const candidateAcceptedTypes = field.FindArray(k_fieldAcceptedTypesHash);
			if (candidateAcceptedTypes != nullptr)
			{
				for (const auto& candidateAcceptedType : *candidateAcceptedTypes)
				{
					if (candidateAcceptedType->GetType() != JSON::ValueType::String)
					{
						AMP_LOG_WARNING("Skipped non-string accepted type in field [%u].", fieldID);
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
		case RecordSchemaFieldType::ImportedType:
		{
			RecordSchemaField importedTypeField =
				RecordSchemaField::MakeImportedTypeField(fieldID, fieldName, fieldDescription);

			const JSON::JSONString* const candidateImportedType = field.FindString(k_fieldImportedTypeHash);
			if (candidateImportedType == nullptr)
			{
				AMP_LOG_WARNING("Skipped non-string imported type in field [%u].", fieldID);
				continue;
			}

			const auto* const importEntry = outSchema.m_importedTypes.Find(candidateImportedType->m_string);
			if (importEntry == nullptr)
			{
				AMP_LOG_WARNING("Skipped imported type in field [%u] because it was not declared in the schema's"
					" imported types.", fieldID);
				continue;
			}

			importedTypeField.m_importedTypeData.m_importedTypeName = candidateImportedType->m_string;

			outSchema.m_fields.Add(std::move(importedTypeField));
			break;
		}
		case RecordSchemaFieldType::Group:
		{
			RecordSchemaField groupField = RecordSchemaField::MakeGroupField(fieldID, fieldName, fieldDescription);

			const JSON::JSONArray* const candidateMemberFieldIDs = field.FindArray(k_fieldMemberIDsHash);
			if (candidateMemberFieldIDs != nullptr)
			{
				for (const auto& candidateMemberFieldID : *candidateMemberFieldIDs)
				{
					if (candidateMemberFieldID->GetType() != JSON::ValueType::Number)
					{
						AMP_LOG_WARNING("Skipped non-number member field ID in field [%u].", fieldID);
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
		case RecordSchemaFieldType::List:
		{
			RecordSchemaField listField = RecordSchemaField::MakeListField(fieldID, fieldName, fieldDescription);

			const JSON::JSONNumber* const candidateElementID = field.FindNumber(k_fieldElementIDHash);
			if (candidateElementID != nullptr)
			{
				listField.m_listData.m_elementFieldID = static_cast<uint16_t>(candidateElementID->m_number);
			}

			outSchema.m_fields.Add(std::move(listField));
			break;
		}
		default:
		{
			AMP_FATAL_ERROR("Encountered unhandled field type [%d].", static_cast<int32_t>(fieldType));
			break;
		}
		}
	}

	// Sort the fields by field ID.
	std::sort(outSchema.m_fields.begin(), outSchema.m_fields.end(),
		[](const RecordSchemaField& lhs, const RecordSchemaField& rhs) { return lhs.m_fieldID < rhs.m_fieldID; });

	return outSchema;
}

RecordSchemaField* RecordSchema::FindField(uint16_t fieldID)
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

const RecordSchemaField* RecordSchema::FindField(uint16_t fieldID) const
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

bool RecordSchema::Accept(RecordSchemaVisitor& visitor, uint16_t fieldID) const
{
	const RecordSchemaField* const field = FindField(fieldID);
	if (field == nullptr)
	{
		return false;
	}

	switch (field->m_type)
	{
	case RecordSchemaFieldType::Invalid:
	{
		return true;
	}
	case RecordSchemaFieldType::Boolean:
	{
		const RecordSchemaVisitor::Flow result = visitor.Visit(*field, field->m_booleanData);
		return (result != RecordSchemaVisitor::Flow::Stop);
	}
	case RecordSchemaFieldType::Float:
	{
		const RecordSchemaVisitor::Flow result = visitor.Visit(*field, field->m_floatData);
		return (result != RecordSchemaVisitor::Flow::Stop);
	}
	case RecordSchemaFieldType::Integer:
	{
		const RecordSchemaVisitor::Flow result = visitor.Visit(*field, field->m_integerData);
		return (result != RecordSchemaVisitor::Flow::Stop);
	}
	case RecordSchemaFieldType::InstanceReference:
	{
		const RecordSchemaVisitor::Flow result = visitor.Visit(*field, field->m_instanceReferenceData);
		return (result != RecordSchemaVisitor::Flow::Stop);
	}
	case RecordSchemaFieldType::ImportedType:
	{
		const RecordSchemaVisitor::Flow result = visitor.Visit(*field, field->m_importedTypeData);
		return (result != RecordSchemaVisitor::Flow::Stop);
	}
	case RecordSchemaFieldType::Group:
	{
		const RecordSchemaVisitor::Flow result = visitor.Visit(*field, field->m_groupData);
		switch (result)
		{
		case RecordSchemaVisitor::Flow::Skip:
		{
			return true;
		}
		case RecordSchemaVisitor::Flow::Stop:
		{
			return false;
		}
		}

		for (const auto& memberFieldID : field->m_groupData.m_memberFieldIDs)
		{
			const bool keepGoing = Accept(visitor, memberFieldID);
			if (!keepGoing)
			{
				return false;
			}
		}
		return true;
	}
	case RecordSchemaFieldType::List:
	{
		const RecordSchemaVisitor::Flow result = visitor.Visit(*field, field->m_listData);
		switch (result)
		{
		case RecordSchemaVisitor::Flow::Skip:
		{
			return true;
		}
		case RecordSchemaVisitor::Flow::Stop:
		{
			return false;
		}
		}

		return Accept(visitor, field->m_listData.m_elementFieldID);
	}
	default:
	{
		AMP_FATAL_ERROR("Unknown field type [%d]", static_cast<int32_t>(field->m_type));
		return false;
	}
	}
}

namespace Internal_RecordSchema
{
class ChildReferenceValidationVisitor : public RecordSchemaVisitor
{
	const uint16_t m_parentID;
	const uint16_t m_childID;

public:
	ChildReferenceValidationVisitor(uint16_t parentID, uint16_t childID)
		: m_parentID(parentID)
		, m_childID(childID)
	{}

	Flow Visit(const RecordSchemaField& field, const RecordSchemaBooleanData& fieldData) override
	{
		return Flow::Visit;
	}

	Flow Visit(const RecordSchemaField& field, const RecordSchemaFloatData& fieldData) override
	{
		return Flow::Visit;
	}

	Flow Visit(const RecordSchemaField& field, const RecordSchemaIntegerData& fieldData) override
	{
		return Flow::Visit;
	}

	Flow Visit(const RecordSchemaField& field, const RecordSchemaInstanceReferenceData& fieldData) override
	{
		return Flow::Visit;
	}

	Flow Visit(const RecordSchemaField& field, const RecordSchemaImportedTypeData& fieldData) override
	{
		return Flow::Visit;
	}

	Flow Visit(const RecordSchemaField& field, const RecordSchemaGroupData& fieldData) override
	{
		if (field.m_fieldID == m_parentID)
		{
			return Flow::Visit;
		}
		for (const auto& memberFieldID : fieldData.m_memberFieldIDs)
		{
			if (memberFieldID == m_childID)
			{
				return Flow::Stop;
			}
		}
		return Flow::Visit;
	}

	Flow Visit(const RecordSchemaField& field, const RecordSchemaListData& fieldData) override
	{
		if (field.m_fieldID != m_parentID && fieldData.m_elementFieldID == m_childID)
		{
			return Flow::Stop;
		}
		return Flow::Visit;
	}
};
}

bool RecordSchema::CheckIsErrorFree() const
{
	using namespace Internal_RecordSchema;

	// Ensure that the schema has a name.
	const char* const schemaName = m_name.c_str();
	if (schemaName == nullptr || schemaName[0] == '\0')
	{
		AMP_LOG_WARNING("!> Schema is missing a name.");
		return false;
	}

	// Ensure the schema has a root group.
	const RecordSchemaField* const rootGroup = FindField(0);
	if (rootGroup == nullptr || rootGroup->m_type != RecordSchemaFieldType::Group)
	{
		AMP_LOG_WARNING("!> Schema is missing a root group.");
		return false;
	}

	// Ensure that all fields have unique IDs.
	for (auto i = m_fields.begin(), fieldsEnd = m_fields.end(); i != fieldsEnd; ++i)
	{
		const uint16_t& fieldID = i->m_fieldID;
		for (auto j = i + 1; j != fieldsEnd; ++j)
		{
			if (fieldID == j->m_fieldID)
			{
				AMP_LOG_WARNING("!> Schema has multiple fields with ID [%u].", fieldID);
				return false;
			}
		}
	}

	// Ensure that all fields have a valid type.
	for (const auto& field : m_fields)
	{
		switch (field.m_type)
		{
		case RecordSchemaFieldType::Boolean:
		case RecordSchemaFieldType::Float:
		case RecordSchemaFieldType::Integer:
		case RecordSchemaFieldType::InstanceReference:
		case RecordSchemaFieldType::ImportedType:
		case RecordSchemaFieldType::Group:
		case RecordSchemaFieldType::List:
		{
			break;
		}
		default:
		{
			AMP_LOG_WARNING("!> Field [%u] has invalid type [%d].", field.m_fieldID,
				static_cast<int32_t>(field.m_type));
			return false;
		}
		}
	}

	// Validate group fields.
	for (const auto& field : m_fields)
	{
		if (field.m_type != RecordSchemaFieldType::Group)
		{
			continue;
		}

		// Ensure that all groups have names.
		const char* const groupName = field.m_fieldName.c_str();
		if (groupName == nullptr || groupName[0] == '\0')
		{
			AMP_LOG_WARNING("!> Group field [%u] is missing a name.", field.m_fieldID);
			return false;
		}

		// Ensure that all group member fields are only referenced by their group.
		for (const auto& memberFieldID : field.m_groupData.m_memberFieldIDs)
		{
			ChildReferenceValidationVisitor visitor{ field.m_fieldID, memberFieldID };
			if (!Accept(visitor, 0))
			{
				AMP_LOG_WARNING("!> Group member field [%u] in group field [%u] is referenced elsewhere.",
					memberFieldID, field.m_fieldID);
				return false;
			}
		}
	}

	// Ensure that all list elements are only referenced by their list.
	for (const auto& field : m_fields)
	{
		if (field.m_type != RecordSchemaFieldType::List)
		{
			continue;
		}

		ChildReferenceValidationVisitor visitor{ field.m_fieldID, field.m_listData.m_elementFieldID };
		if (!Accept(visitor, 0))
		{
			AMP_LOG_WARNING("!> List element field [%u] in list field [%u] is referenced elsewhere.",
				field.m_listData.m_elementFieldID, field.m_fieldID);
			return false;
		}
	}

	return true;
}
}
