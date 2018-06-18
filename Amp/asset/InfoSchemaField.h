#pragma once

#include <collection/Vector.h>

#include <cstdint>
#include <string>

namespace Asset
{
/**
* A definition of a field within a schema.
*/
struct InfoSchemaField;

enum class InfoSchemaFieldType : uint8_t
{
	Invalid = 0,
	Boolean,
	Float,
	Integer,
	InstanceReference,
	Group
};

struct InfoSchemaBooleanData
{
	bool m_defaultValue;
};

struct InfoSchemaFloatData
{
	float m_defaultValue;
	float m_minimumValue;
	float m_maximumValue;
};

struct InfoSchemaIntegerData
{
	int32_t m_defaultValue;
	int32_t m_minimumValue;
	int32_t m_maximumValue;
};

struct InfoSchemaInstanceReferenceData
{
	Collection::Vector<std::string> m_acceptedTypes;
};

struct InfoSchemaGroupData
{
	Collection::Vector<uint16_t> m_memberFieldIDs;
};

struct InfoSchemaField
{
	InfoSchemaFieldType m_type{ InfoSchemaFieldType::Invalid };
	uint16_t m_fieldID{ UINT16_MAX };
	union
	{
		InfoSchemaBooleanData m_booleanData;
		InfoSchemaFloatData m_floatData;
		InfoSchemaIntegerData m_integerData;
		InfoSchemaInstanceReferenceData m_instanceReferenceData;
		InfoSchemaGroupData m_groupData;
	};

	InfoSchemaField();
	~InfoSchemaField();

	InfoSchemaField(const InfoSchemaField& o) = delete;
	void operator=(const InfoSchemaField& rhs) = delete;

	InfoSchemaField(InfoSchemaField&& o);
	void operator=(InfoSchemaField&& rhs);

	static InfoSchemaField MakeBooleanField(const uint16_t fieldID);
	static InfoSchemaField MakeFloatField(const uint16_t fieldID);
	static InfoSchemaField MakeIntegerField(const uint16_t fieldID);
	static InfoSchemaField MakeInstanceReferenceField(const uint16_t fieldID);
	static InfoSchemaField MakeGroupField(const uint16_t fieldID);
};
}
