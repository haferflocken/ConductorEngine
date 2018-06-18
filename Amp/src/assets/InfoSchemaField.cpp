#include <asset/InfoSchemaField.h>

#include <dev/Dev.h>

namespace Asset
{
namespace Internal_InfoSchemaField
{
void MoveData(InfoSchemaField& destination, InfoSchemaField&& source)
{
	switch (destination.m_type)
	{
	case InfoSchemaFieldType::Invalid:
	{
		break;
	}
	case InfoSchemaFieldType::Boolean:
	{
		destination.m_booleanData = std::move(source.m_booleanData);
		break;
	}
	case InfoSchemaFieldType::Float:
	{
		destination.m_floatData = std::move(source.m_floatData);
		break;
	}
	case InfoSchemaFieldType::Integer:
	{
		destination.m_integerData = std::move(source.m_integerData);
		break;
	}
	case InfoSchemaFieldType::InstanceReference:
	{
		destination.m_instanceReferenceData = std::move(source.m_instanceReferenceData);
		break;
	}
	case InfoSchemaFieldType::Group:
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

void DestroyInfoSchemaField(InfoSchemaField& field)
{
	switch (field.m_type)
	{
	case InfoSchemaFieldType::Invalid:
	{
		break;
	}
	case InfoSchemaFieldType::Boolean:
	{
		MoveDestroy(field.m_booleanData);
		break;
	}
	case InfoSchemaFieldType::Float:
	{
		MoveDestroy(field.m_floatData);
		break;
	}
	case InfoSchemaFieldType::Integer:
	{
		MoveDestroy(field.m_integerData);
		break;
	}
	case InfoSchemaFieldType::InstanceReference:
	{
		MoveDestroy(field.m_instanceReferenceData);
		break;
	}
	case InfoSchemaFieldType::Group:
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

InfoSchemaField::InfoSchemaField()
{}

InfoSchemaField::~InfoSchemaField()
{
	Internal_InfoSchemaField::DestroyInfoSchemaField(*this);
	m_type = InfoSchemaFieldType::Invalid;
}

InfoSchemaField::InfoSchemaField(InfoSchemaField&& o)
	: m_type(o.m_type)
	, m_fieldID(o.m_fieldID)
{
	Internal_InfoSchemaField::MoveData(*this, std::move(o));
}

void InfoSchemaField::operator=(InfoSchemaField&& rhs)
{
	Internal_InfoSchemaField::DestroyInfoSchemaField(*this);

	m_type = rhs.m_type;
	m_fieldID = rhs.m_fieldID;

	Internal_InfoSchemaField::MoveData(*this, std::move(rhs));
}

InfoSchemaField InfoSchemaField::MakeBooleanField(const uint16_t fieldID)
{
	InfoSchemaField field;
	field.m_type = InfoSchemaFieldType::Boolean;
	field.m_fieldID = fieldID;

	field.m_booleanData.m_defaultValue = false;

	return field;
}

InfoSchemaField InfoSchemaField::MakeFloatField(const uint16_t fieldID)
{
	InfoSchemaField field;
	field.m_type = InfoSchemaFieldType::Float;
	field.m_fieldID = fieldID;

	field.m_floatData.m_defaultValue = 0.0f;
	field.m_floatData.m_minimumValue = -FLT_MAX;
	field.m_floatData.m_maximumValue = FLT_MAX;

	return field;
}

InfoSchemaField InfoSchemaField::MakeIntegerField(const uint16_t fieldID)
{
	InfoSchemaField field;
	field.m_type = InfoSchemaFieldType::Integer;
	field.m_fieldID = fieldID;

	field.m_integerData.m_defaultValue = 0;
	field.m_integerData.m_minimumValue = INT32_MIN;
	field.m_integerData.m_maximumValue = INT32_MAX;

	return field;
}

InfoSchemaField InfoSchemaField::MakeInstanceReferenceField(const uint16_t fieldID)
{
	InfoSchemaField field;
	field.m_type = InfoSchemaFieldType::InstanceReference;
	field.m_fieldID = fieldID;

	new(&field.m_instanceReferenceData) InfoSchemaInstanceReferenceData();

	return field;
}

InfoSchemaField InfoSchemaField::MakeGroupField(const uint16_t fieldID)
{
	InfoSchemaField field;
	field.m_type = InfoSchemaFieldType::Group;
	field.m_fieldID = fieldID;

	new(&field.m_groupData) InfoSchemaGroupData();

	return field;
}
}
