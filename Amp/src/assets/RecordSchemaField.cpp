#include <asset/RecordSchemaField.h>

#include <dev/Dev.h>

namespace Asset
{
namespace Internal_RecordSchemaField
{
void ConstructData(RecordSchemaField& destination, RecordSchemaField&& source)
{
	switch (destination.m_type)
	{
	case RecordSchemaFieldType::Invalid:
	{
		break;
	}
	case RecordSchemaFieldType::Boolean:
	{
		new (&destination.m_booleanData) RecordSchemaBooleanData(std::move(source.m_booleanData));
		break;
	}
	case RecordSchemaFieldType::Float:
	{
		new (&destination.m_floatData) RecordSchemaFloatData(std::move(source.m_floatData));
		break;
	}
	case RecordSchemaFieldType::Integer:
	{
		new (&destination.m_integerData) RecordSchemaIntegerData(std::move(source.m_integerData));
		break;
	}
	case RecordSchemaFieldType::InstanceReference:
	{
		new (&destination.m_instanceReferenceData) RecordSchemaInstanceReferenceData(std::move(source.m_instanceReferenceData));
		break;
	}
	case RecordSchemaFieldType::ImportedType:
	{
		new (&destination.m_importedTypeData) RecordSchemaImportedTypeData(std::move(source.m_importedTypeData));
		break;
	}
	case RecordSchemaFieldType::Group:
	{
		new (&destination.m_groupData) RecordSchemaGroupData(std::move(source.m_groupData));
		break;
	}
	case RecordSchemaFieldType::List:
	{
		new (&destination.m_listData) RecordSchemaListData(std::move(source.m_listData));
		break;
	}
	default:
	{
		AMP_FATAL_ERROR("Unknown field type [%d]", static_cast<int32_t>(destination.m_type));
		break;
	}
	}
}

void MoveData(RecordSchemaField& destination, RecordSchemaField&& source)
{
	switch (destination.m_type)
	{
	case RecordSchemaFieldType::Invalid:
	{
		break;
	}
	case RecordSchemaFieldType::Boolean:
	{
		destination.m_booleanData = std::move(source.m_booleanData);
		break;
	}
	case RecordSchemaFieldType::Float:
	{
		destination.m_floatData = std::move(source.m_floatData);
		break;
	}
	case RecordSchemaFieldType::Integer:
	{
		destination.m_integerData = std::move(source.m_integerData);
		break;
	}
	case RecordSchemaFieldType::InstanceReference:
	{
		destination.m_instanceReferenceData = std::move(source.m_instanceReferenceData);
		break;
	}
	case RecordSchemaFieldType::ImportedType:
	{
		destination.m_importedTypeData = std::move(source.m_importedTypeData);
		break;
	}
	case RecordSchemaFieldType::Group:
	{
		destination.m_groupData = std::move(source.m_groupData);
		break;
	}
	case RecordSchemaFieldType::List:
	{
		destination.m_listData = std::move(source.m_listData);
		break;
	}
	default:
	{
		AMP_FATAL_ERROR("Unknown field type [%d]", static_cast<int32_t>(destination.m_type));
		break;
	}
	}
}

template <typename T>
void MoveDestroy(T& data)
{
	const T toDestroy = std::move(data);
}

void DestroyInfoSchemaField(RecordSchemaField& field)
{
	switch (field.m_type)
	{
	case RecordSchemaFieldType::Invalid:
	{
		break;
	}
	case RecordSchemaFieldType::Boolean:
	{
		MoveDestroy(field.m_booleanData);
		break;
	}
	case RecordSchemaFieldType::Float:
	{
		MoveDestroy(field.m_floatData);
		break;
	}
	case RecordSchemaFieldType::Integer:
	{
		MoveDestroy(field.m_integerData);
		break;
	}
	case RecordSchemaFieldType::InstanceReference:
	{
		MoveDestroy(field.m_instanceReferenceData);
		break;
	}
	case RecordSchemaFieldType::ImportedType:
	{
		MoveDestroy(field.m_importedTypeData);
		break;
	}
	case RecordSchemaFieldType::Group:
	{
		MoveDestroy(field.m_groupData);
		break;
	}
	case RecordSchemaFieldType::List:
	{
		MoveDestroy(field.m_listData);
		break;
	}
	default:
	{
		AMP_FATAL_ERROR("Unknown field type [%d]", static_cast<int32_t>(field.m_type));
		break;
	}
	}
}
}

RecordSchemaField::RecordSchemaField()
{}

RecordSchemaField::~RecordSchemaField()
{
	Internal_RecordSchemaField::DestroyInfoSchemaField(*this);
	m_type = RecordSchemaFieldType::Invalid;
}

RecordSchemaField::RecordSchemaField(RecordSchemaField&& o)
	: m_type(o.m_type)
	, m_fieldID(o.m_fieldID)
	, m_fieldName(std::move(o.m_fieldName))
	, m_fieldDescription(std::move(o.m_fieldDescription))
{
	Internal_RecordSchemaField::ConstructData(*this, std::move(o));
}

void RecordSchemaField::operator=(RecordSchemaField&& rhs)
{
	Internal_RecordSchemaField::DestroyInfoSchemaField(*this);

	m_type = rhs.m_type;
	m_fieldID = rhs.m_fieldID;
	m_fieldName = std::move(rhs.m_fieldName);
	m_fieldDescription = std::move(rhs.m_fieldDescription);

	Internal_RecordSchemaField::MoveData(*this, std::move(rhs));
}

RecordSchemaField RecordSchemaField::MakeBooleanField(uint16_t fieldID, const char* name, const char* description)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::Boolean;
	field.m_fieldID = fieldID;
	field.m_fieldName = name;
	field.m_fieldDescription = description;

	field.m_booleanData.m_defaultValue = false;

	return field;
}

RecordSchemaField RecordSchemaField::MakeFloatField(uint16_t fieldID, const char* name, const char* description)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::Float;
	field.m_fieldID = fieldID;
	field.m_fieldName = name;
	field.m_fieldDescription = description;

	field.m_floatData.m_defaultValue = 0.0f;
	field.m_floatData.m_minimumValue = -FLT_MAX;
	field.m_floatData.m_maximumValue = FLT_MAX;

	return field;
}

RecordSchemaField RecordSchemaField::MakeIntegerField(uint16_t fieldID, const char* name, const char* description)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::Integer;
	field.m_fieldID = fieldID;
	field.m_fieldName = name;
	field.m_fieldDescription = description;

	field.m_integerData.m_defaultValue = 0;
	field.m_integerData.m_minimumValue = INT32_MIN;
	field.m_integerData.m_maximumValue = INT32_MAX;

	return field;
}

RecordSchemaField RecordSchemaField::MakeInstanceReferenceField(
	uint16_t fieldID, const char* name, const char* description)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::InstanceReference;
	field.m_fieldID = fieldID;
	field.m_fieldName = name;
	field.m_fieldDescription = description;

	new(&field.m_instanceReferenceData) RecordSchemaInstanceReferenceData();

	return field;
}

RecordSchemaField RecordSchemaField::MakeImportedTypeField(
	uint16_t fieldID, const char* name, const char* description)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::ImportedType;
	field.m_fieldID = fieldID;
	field.m_fieldName = name;
	field.m_fieldDescription = description;

	new(&field.m_importedTypeData) RecordSchemaImportedTypeData();

	return field;
}

RecordSchemaField RecordSchemaField::MakeGroupField(uint16_t fieldID, const char* name, const char* description)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::Group;
	field.m_fieldID = fieldID;
	field.m_fieldName = name;
	field.m_fieldDescription = description;

	new(&field.m_groupData) RecordSchemaGroupData();

	return field;
}

RecordSchemaField RecordSchemaField::MakeListField(uint16_t fieldID, const char* name, const char* description)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::List;
	field.m_fieldID = fieldID;
	field.m_fieldName = name;
	field.m_fieldDescription = description;

	field.m_listData.m_elementFieldID = UINT16_MAX;

	return field;
}
}
