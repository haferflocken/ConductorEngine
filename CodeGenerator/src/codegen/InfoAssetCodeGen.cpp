#include <codegen/InfoAssetCodeGen.h>

#include <asset/RecordSchema.h>
#include <asset/RecordSchemaVisitor.h>
#include <codegen/CppStream.h>
#include <collection/VectorMap.h>
#include <dev/Dev.h>

namespace Internal_InfoAssetCodeGen
{
class WriteInfoInstanceStructVisitor : public Asset::RecordSchemaVisitor
{
	const Asset::RecordSchema& m_schema;
	CodeGen::CppStream& m_output;
	const uint16_t m_rootFieldID;

public:
	explicit WriteInfoInstanceStructVisitor(
		const Asset::RecordSchema& schema,
		CodeGen::CppStream& output,
		uint16_t rootFieldID)
		: m_schema(schema)
		, m_output(output)
		, m_rootFieldID(rootFieldID)
	{}

	virtual ~WriteInfoInstanceStructVisitor() {}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaBooleanData& fieldData) override
	{
		m_output.NewLine();
		m_output << "bool " << field.m_fieldName.c_str() << ";";
		return Flow::Visit;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaFloatData& fieldData) override
	{
		m_output.NewLine();
		m_output << "float " << field.m_fieldName.c_str() << ";";
		return Flow::Visit;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaIntegerData& fieldData) override
	{
		m_output.NewLine();
		m_output << "int32_t " << field.m_fieldName.c_str() << ";";
		return Flow::Visit;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaInstanceReferenceData& fieldData) override
	{
		m_output.NewLine();
		m_output << "std::string " << field.m_fieldName.c_str() << ";";
		return Flow::Visit;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaGroupData& fieldData) override
	{
		// If this group is the root field, it does not need a struct generated for it.
		if (field.m_fieldID == m_rootFieldID)
		{
			// Visit all the fields within the group.
			return Flow::Visit;
		}

		// Write out an inner struct for the group with an uppercase first letter.
		const char* const groupName = field.m_fieldName.c_str();
		if (groupName == nullptr || groupName[0] == '\0')
		{
			Dev::LogWarning("Cannot generate an inner struct for an unnamed group.");
			return Flow::Skip;
		}
		const char upperFirst = static_cast<char>(toupper(groupName[0]));

		m_output.NewLine();
		m_output << "struct " << upperFirst << (groupName + 1);
		m_output.NewLine();
		m_output << "{";

		m_output.Indent([&]()
		{
			// Treat this group as the root of another visitor so that the schema handles traversing the member fields.
			WriteInfoInstanceStructVisitor subVisitor{ m_schema, m_output, field.m_fieldID };
			m_schema.Accept(subVisitor, field.m_fieldID);
		});
		m_output.NewLine();
		m_output << "};";

		// Create a variable of the struct with a lowercase first letter.
		const char lowerFirst = static_cast<char>(tolower(groupName[0]));
		m_output << upperFirst << (groupName + 1) << " " << lowerFirst << (groupName + 1) << ";";

		// Do not visit the fields within the group because they were visited by the subVisitor.
		return Flow::Skip;
	}
};
}

std::string CodeGen::GenerateInfoInstanceStruct(const Asset::RecordSchema& schema)
{
	using namespace Internal_InfoAssetCodeGen;

	// Write out the instance struct.
	CppStream output;
	output << "#include <cstdint>\n";
	output << "#include <string>\n";
	output << "struct " << schema.GetName().c_str() << "\n{";

	output.Indent([&]()
	{
		WriteInfoInstanceStructVisitor writeVisitor{ schema, output, 0 };
		schema.Accept(writeVisitor, 0);
	});

	output.NewLine();
	output << "}\n";
	return output.CopyOut();
}

std::string CodeGen::GenerateInfoInstanceSaveFunction(const Asset::RecordSchema& schema)
{
	const Asset::RecordSchemaField* const rootGroup = schema.FindField(0);
	if (rootGroup == nullptr || rootGroup->m_type != Asset::RecordSchemaFieldType::Group)
	{
		return "";
	}
	const Asset::RecordSchemaGroupData& rootGroupData = rootGroup->m_groupData;

	CppStream output;
	output << "JSON::JSONObject SaveInfoInstance(";
	output << "const " << schema.GetName().c_str() << "& infoInstance";
	output << ")\n{";

	output.Indent([&]()
	{
		// TODO
	});

	output.NewLine();
	output << "}\n";
	return output.CopyOut();
}

std::string CodeGen::GenerateInfoInstanceLoadFunction(const Asset::RecordSchema& schema)
{
	// TODO
	return "";
}
