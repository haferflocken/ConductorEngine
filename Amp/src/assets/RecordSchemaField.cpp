#include <asset/RecordSchemaField.h>

#include <dev/Dev.h>

namespace Asset
{
namespace Internal_RecordSchemaField
{
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
	case RecordSchemaFieldType::Group:
	{
		destination.m_groupData = std::move(source.m_groupData);
		break;
	}
	default:
	{
		Dev::FatalError("Unknown field type [%d]", static_cast<int32_t>(destination.m_type));
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
	case RecordSchemaFieldType::Group:
	{
		MoveDestroy(field.m_groupData);
		break;
	}
	default:
	{
		Dev::FatalError("Unknown field type [%d]", static_cast<int32_t>(field.m_type));
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
{
	Internal_RecordSchemaField::MoveData(*this, std::move(o));
}

void RecordSchemaField::operator=(RecordSchemaField&& rhs)
{
	Internal_RecordSchemaField::DestroyInfoSchemaField(*this);

	m_type = rhs.m_type;
	m_fieldID = rhs.m_fieldID;

	Internal_RecordSchemaField::MoveData(*this, std::move(rhs));
}

RecordSchemaField RecordSchemaField::MakeBooleanField(const uint16_t fieldID)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::Boolean;
	field.m_fieldID = fieldID;

	field.m_booleanData.m_defaultValue = false;

	return field;
}

RecordSchemaField RecordSchemaField::MakeFloatField(const uint16_t fieldID)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::Float;
	field.m_fieldID = fieldID;

	field.m_floatData.m_defaultValue = 0.0f;
	field.m_floatData.m_minimumValue = -FLT_MAX;
	field.m_floatData.m_maximumValue = FLT_MAX;

	return field;
}

RecordSchemaField RecordSchemaField::MakeIntegerField(const uint16_t fieldID)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::Integer;
	field.m_fieldID = fieldID;

	field.m_integerData.m_defaultValue = 0;
	field.m_integerData.m_minimumValue = INT32_MIN;
	field.m_integerData.m_maximumValue = INT32_MAX;

	return field;
}

RecordSchemaField RecordSchemaField::MakeInstanceReferenceField(const uint16_t fieldID)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::InstanceReference;
	field.m_fieldID = fieldID;

	new(&field.m_instanceReferenceData) RecordSchemaInstanceReferenceData();

	return field;
}

RecordSchemaField RecordSchemaField::MakeGroupField(const uint16_t fieldID)
{
	RecordSchemaField field;
	field.m_type = RecordSchemaFieldType::Group;
	field.m_fieldID = fieldID;

	new(&field.m_groupData) RecordSchemaGroupData();

	return field;
}
}
