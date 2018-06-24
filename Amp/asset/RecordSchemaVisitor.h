#pragma once

namespace Asset
{
struct RecordSchemaBooleanData;
struct RecordSchemaFloatData;
struct RecordSchemaIntegerData;
struct RecordSchemaInstanceReferenceData;
struct RecordSchemaGroupData;
struct RecordSchemaListData;
struct RecordSchemaField;

/**
 * A visitor for traversing a RecordSchema.
 */
class RecordSchemaVisitor
{
public:
	enum class Flow
	{
		Visit,
		Skip,
		Stop
	};

	virtual ~RecordSchemaVisitor() {}

	virtual Flow Visit(const RecordSchemaField& field, const RecordSchemaBooleanData& fieldData) = 0;
	virtual Flow Visit(const RecordSchemaField& field, const RecordSchemaFloatData& fieldData) = 0;
	virtual Flow Visit(const RecordSchemaField& field, const RecordSchemaIntegerData& fieldData) = 0;
	virtual Flow Visit(const RecordSchemaField& field, const RecordSchemaInstanceReferenceData& fieldData) = 0;
	virtual Flow Visit(const RecordSchemaField& field, const RecordSchemaGroupData& fieldData) = 0;
	virtual Flow Visit(const RecordSchemaField& field, const RecordSchemaListData& fieldData) = 0;
};
}
