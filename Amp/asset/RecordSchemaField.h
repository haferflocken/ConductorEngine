#pragma once

#include <collection/Vector.h>

#include <cstdint>
#include <string>

namespace Asset
{
class RecordSchemaVisitor;

/**
* A definition of a field within a schema.
*/
struct RecordSchemaField;

enum class RecordSchemaFieldType : uint8_t
{
	Invalid = 0,
	Boolean,
	Float,
	Integer,
	InstanceReference,
	ImportedType,
	Group,
	List
};

struct RecordSchemaBooleanData
{
	bool m_defaultValue;
};

struct RecordSchemaFloatData
{
	float m_defaultValue;
	float m_minimumValue;
	float m_maximumValue;
};

struct RecordSchemaIntegerData
{
	int32_t m_defaultValue;
	int32_t m_minimumValue;
	int32_t m_maximumValue;
};

struct RecordSchemaInstanceReferenceData
{
	Collection::Vector<std::string> m_acceptedTypes;
};

struct RecordSchemaImportedTypeData
{
	std::string m_importedTypeName;
};

struct RecordSchemaGroupData
{
	Collection::Vector<uint16_t> m_memberFieldIDs;
};

struct RecordSchemaListData
{
	uint16_t m_elementFieldID;
};

struct RecordSchemaField
{
	RecordSchemaFieldType m_type{ RecordSchemaFieldType::Invalid };
	uint16_t m_fieldID{ UINT16_MAX };
	std::string m_fieldName{};
	std::string m_fieldDescription{};
	union
	{
		RecordSchemaBooleanData m_booleanData;
		RecordSchemaFloatData m_floatData;
		RecordSchemaIntegerData m_integerData;
		RecordSchemaInstanceReferenceData m_instanceReferenceData;
		RecordSchemaImportedTypeData m_importedTypeData;
		RecordSchemaGroupData m_groupData;
		RecordSchemaListData m_listData;
	};

	RecordSchemaField();
	~RecordSchemaField();

	RecordSchemaField(const RecordSchemaField& o) = delete;
	void operator=(const RecordSchemaField& rhs) = delete;

	RecordSchemaField(RecordSchemaField&& o);
	void operator=(RecordSchemaField&& rhs);

	static RecordSchemaField MakeBooleanField(uint16_t fieldID, const char* name, const char* description);
	static RecordSchemaField MakeFloatField(uint16_t fieldID, const char* name, const char* description);
	static RecordSchemaField MakeIntegerField(uint16_t fieldID, const char* name, const char* description);
	static RecordSchemaField MakeInstanceReferenceField(uint16_t fieldID, const char* name, const char* description);
	static RecordSchemaField MakeImportedTypeField(uint16_t fieldID, const char* name, const char* description);
	static RecordSchemaField MakeGroupField(uint16_t fieldID, const char* name, const char* description);
	static RecordSchemaField MakeListField(uint16_t fieldID, const char* name, const char* description);
};
}
