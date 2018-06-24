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
		WriteVariable("bool", field.m_fieldName.c_str(), fieldData.m_defaultValue ? "true" : "false" );
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
		Dev::FatalAssert(groupName != nullptr && groupName[0] != '\0', "Group field [%u] is missing a name.",
			field.m_fieldID);

		WriteGroupStruct(field, groupName);

		// Create a variable of the struct with a lowercase first letter.
		const char lowerFirst = static_cast<char>(tolower(groupName[0]));

		m_output.NewLine();
		m_output.AppendCapitalized(groupName);
		m_output << " " << lowerFirst << (groupName + 1) << ";";

		// Do not visit the fields within the group because they were visited by the subVisitor.
		return Flow::Skip;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaListData& fieldData) override
	{
		std::string typeString = "Collection::Vector<";
		std::string typeStringSuffix = ">";
		if (!TryCalcListElementTypeAndWriteDependentTypes(fieldData.m_elementFieldID, typeString, typeStringSuffix))
		{
			Dev::LogWarning("!> Unable to generate list field [%u].", field.m_fieldID);
			return Flow::Skip;
		}
		typeString += typeStringSuffix;

		WriteVariable(typeString.c_str(), field.m_fieldName.c_str(), "");

		// Do not visit the element field of the list because it was already visited when determining the type string.
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

	void WriteGroupStruct(const Asset::RecordSchemaField& field, const char* const groupName)
	{
		m_output.NewLine();
		m_output << "struct ";
		m_output.AppendCapitalized(groupName);
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
	}

	bool TryCalcListElementTypeAndWriteDependentTypes(
		const uint16_t elementFieldID,
		std::string& outType,
		std::string& outTypeSuffix)
	{
		// Write out a struct for the list's element if one is needed.
		const Asset::RecordSchemaField* const elementField = m_schema.FindField(elementFieldID);
		if (elementField == nullptr)
		{
			Dev::LogWarning("!> Cannot generate a variable for a list with no element field.");
			return false;
		}

		switch (elementField->m_type)
		{
		case Asset::RecordSchemaFieldType::Boolean:
		{
			outType += "bool";
			return true;
		}
		case Asset::RecordSchemaFieldType::Float:
		{
			outType += "float";
			return true;
		}
		case Asset::RecordSchemaFieldType::Integer:
		{
			outType += "int32_t";
			return true;
		}
		case Asset::RecordSchemaFieldType::InstanceReference:
		{
			const auto& acceptedTypes = elementField->m_instanceReferenceData.m_acceptedTypes;
			if (acceptedTypes.IsEmpty())
			{
				outType += "void";
			}
			else
			{
				outType += "std::string";
			}
			return true;
		}
		case Asset::RecordSchemaFieldType::Group:
		{
			// Write out an inner struct for the group with an uppercase first letter.
			const char* const groupName = elementField->m_fieldName.c_str();
			Dev::FatalAssert(groupName != nullptr && groupName[0] != '\0', "Group field [%u] is missing a name.",
				elementField->m_fieldID);

			WriteGroupStruct(*elementField, groupName);

			// Copy the group struct name to outType.
			outType += static_cast<char>(toupper(groupName[0]));
			outType += (groupName + 1);
			return true;
		}
		case Asset::RecordSchemaFieldType::List:
		{
			outType += "Collection::Vector<";
			outTypeSuffix += ">";

			return TryCalcListElementTypeAndWriteDependentTypes(elementField->m_listData.m_elementFieldID,
				outType, outTypeSuffix);
		}
		default:
		{
			Dev::FatalError("Field [%u] has unknown type [%d]", elementFieldID,
				static_cast<int32_t>(elementField->m_type));
			return false;
		}
		}
	}
};

void WriteNamespaceDeclaration(
	const std::string* namespaceNames,
	const size_t numNamespaceNames,
	CodeGen::CppStream& output)
{
	output << "namespace " << namespaceNames[0].c_str();
	for (size_t i = 1; i < numNamespaceNames; ++i)
	{
		output << "::" << namespaceNames[i].c_str();
	}
}
}

void CodeGen::GenerateInfoInstanceStruct(
	const std::string* namespaceNames,
	const size_t numNamespaceNames,
	const Asset::RecordSchema& schema,
	std::ostream& outputStream)
{
	using namespace Internal_InfoAssetCodeGen;

	const char* const name = schema.GetName().c_str();
	if (name == nullptr || name[0] == '\0')
	{
		return;
	}

	CppStream output{ outputStream };
	output << "// GENERATED CODE\n";

	// Write out the required includes.
	output << "#include <collection/Vector.h>\n";
	output << "#include <cstdint>\n";
	output << "#include <string>\n";

	// Write out the namespaces.
	output.NewLine();
	WriteNamespaceDeclaration(namespaceNames, numNamespaceNames, output);
	output << "\n{";
	
	// Write out the instance struct.
	output.NewLine();
	output << "struct ";
	output.AppendCapitalized(name);
	output << "\n{";

	output.Indent([&]()
	{
		WriteInfoInstanceStructVisitor writeVisitor{ schema, output, 0 };
		schema.Accept(writeVisitor, 0);
	});

	output.NewLine();
	output << "};\n";

	// Close the namespaces brace.
	output << "}\n";
}

void CodeGen::GenerateInfoInstanceSaveFunction(
	const std::string* namespaceNames,
	const size_t numNamespaceNames,
	const Asset::RecordSchema& schema,
	std::ostream& outputStream)
{
	using namespace Internal_InfoAssetCodeGen;

	const char* const name = schema.GetName().c_str();
	if (name == nullptr || name[0] == '\0')
	{
		return;
	}

	CppStream output{ outputStream };
	output << "// GENERATED CODE\n";

	// Write out the required includes.
	output << "#include <json/JSONTypes.h>\n";

	// Write out the namespaces.
	output.NewLine();
	WriteNamespaceDeclaration(namespaceNames, numNamespaceNames, output);
	output << "\n{";

	// Write out the save function.
	output.NewLine();
	output << "JSON::JSONObject SaveInfoInstance(const ";
	output.AppendCapitalized(name);
	output << "& infoInstance)\n{";

	output.Indent([&]()
	{
		// TODO
		output.NewLine();
		output << "return JSON::JSONObject();";
	});

	output.NewLine();
	output << "}\n";

	// Close the namespaces brace.
	output << "}\n";
}

void CodeGen::GenerateInfoInstanceLoadFunction(
	const std::string* namespaceNames,
	const size_t numNamespaceNames,
	const Asset::RecordSchema& schema,
	std::ostream& outputStream)
{
	// TODO
}
