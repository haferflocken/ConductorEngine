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
		WriteVariable("bool", field.m_fieldName.c_str(), std::to_string(fieldData.m_defaultValue).c_str());
		return Flow::Visit;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaFloatData& fieldData) override
	{
		WriteVariable("float", field.m_fieldName.c_str(), std::to_string(fieldData.m_defaultValue).c_str());
		return Flow::Visit;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaIntegerData& fieldData) override
	{
		WriteVariable("int32_t", field.m_fieldName.c_str(), std::to_string(fieldData.m_defaultValue).c_str());
		return Flow::Visit;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaInstanceReferenceData& fieldData) override
	{
		WriteVariable("std::string", field.m_fieldName.c_str(), "");
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

		m_output.NewLine();
		m_output << upperFirst << (groupName + 1) << " " << lowerFirst << (groupName + 1) << ";";

		// Do not visit the fields within the group because they were visited by the subVisitor.
		return Flow::Skip;
	}

private:
	void WriteVariable(const char* const type, const char* const rawName, const char* const defaultValue)
	{
		const char lowerFirst = static_cast<char>(rawName[0]);

		m_output.NewLine();
		m_output << type << " m_" << lowerFirst << (rawName + 1);
		m_output << "{ " << defaultValue << " };";
	}
};
}

std::string CodeGen::GenerateInfoInstanceStruct(
	const std::string* namespaceNames,
	const size_t numNamespaceNames,
	const Asset::RecordSchema& schema)
{
	using namespace Internal_InfoAssetCodeGen;

	const char* const name = schema.GetName().c_str();
	if (name == nullptr || name[0] == '\0')
	{
		return "";
	}
	const char upperFirst = static_cast<char>(toupper(name[0]));

	// Write out the required includes.
	CppStream output;
	output << "#include <cstdint>\n";
	output << "#include <string>\n";

	// Write out the namespaces.
	output.NewLine();
	output << "namespace " << namespaceNames[0].c_str();
	for (size_t i = 1; i < numNamespaceNames; ++i)
	{
		output << "::" << namespaceNames[i].c_str();
	}
	output << "\n{";
	
	// Write out the instance struct.
	output.NewLine();
	output << "struct " << upperFirst << (name + 1) << "\n{";

	output.Indent([&]()
	{
		WriteInfoInstanceStructVisitor writeVisitor{ schema, output, 0 };
		schema.Accept(writeVisitor, 0);
	});

	output.NewLine();
	output << "};\n";

	// Close the namespaces brace and return.
	output << "}\n";
	return output.CopyOut();
}

std::string CodeGen::GenerateInfoInstanceSaveFunction(
	const std::string* namespaceNames,
	const size_t numNamespaceNames,
	const Asset::RecordSchema& schema)
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

std::string CodeGen::GenerateInfoInstanceLoadFunction(
	const std::string* namespaceNames,
	const size_t numNamespaceNames,
	const Asset::RecordSchema& schema)
{
	// TODO
	return "";
}
