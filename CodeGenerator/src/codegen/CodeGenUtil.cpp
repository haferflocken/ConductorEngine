#include <codegen/CodeGenUtil.h>

#include <asset/RecordSchema.h>
#include <asset/RecordSchemaVisitor.h>

#include <codegen/CppStream.h>

void CodeGen::WriteNamespaceDeclaration(
	const Collection::ArrayView<std::string>& namespaceNames,
	CodeGen::CppStream& output)
{
	output << "namespace " << namespaceNames[namespaceNames.Size() - 1].c_str();
	for (int64_t i = namespaceNames.Size() -  2; i >= 0; --i)
	{
		output << "::" << namespaceNames[i].c_str();
	}
}


namespace Internal_CodeGenUtil
{
class WriteStructMembersVisitor : public Asset::RecordSchemaVisitor
{
	const Asset::RecordSchema& m_schema;
	CodeGen::CppStream& m_output;
	const uint16_t m_rootFieldID;

public:
	WriteStructMembersVisitor(
		const Asset::RecordSchema& schema,
		CodeGen::CppStream& output,
		uint16_t rootFieldID)
		: m_schema(schema)
		, m_output(output)
		, m_rootFieldID(rootFieldID)
	{}

	virtual ~WriteStructMembersVisitor() {}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaBooleanData& fieldData) override
	{
		WriteVariableDeclaration("bool", field.m_fieldName.c_str(), field.m_fieldDescription.c_str(),
			fieldData.m_defaultValue ? "true" : "false");
		return Flow::Visit;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaFloatData& fieldData) override
	{
		WriteVariableDeclaration("float", field.m_fieldName.c_str(), field.m_fieldDescription.c_str(),
			std::to_string(fieldData.m_defaultValue).c_str());
		return Flow::Visit;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaIntegerData& fieldData) override
	{
		WriteVariableDeclaration("int32_t", field.m_fieldName.c_str(), field.m_fieldDescription.c_str(),
			std::to_string(fieldData.m_defaultValue).c_str());
		return Flow::Visit;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaInstanceReferenceData& fieldData) override
	{
		WriteVariableDeclaration("std::string", field.m_fieldName.c_str(), field.m_fieldDescription.c_str(), "");
		return Flow::Visit;
	}

	Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaImportedTypeData& fieldData) override
	{
		WriteVariableDeclaration(fieldData.m_importedTypeName.c_str(), field.m_fieldName.c_str(),
			field.m_fieldDescription.c_str(), "");
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
		AMP_FATAL_ASSERT(groupName != nullptr && groupName[0] != '\0', "Group field [%u] is missing a name.",
			field.m_fieldID);

		WriteGroupStruct(field, groupName);

		// Create a variable of the struct with a lowercase first letter.
		const char lowerFirst = static_cast<char>(tolower(groupName[0]));

		m_output.NewLine();
		m_output.AppendCapitalized(groupName);
		m_output << " m_" << lowerFirst << (groupName + 1) << ";";

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

		WriteVariableDeclaration(typeString.c_str(), field.m_fieldName.c_str(), field.m_fieldDescription.c_str(), "");

		// Do not visit the element field of the list because it was already visited when determining the type string.
		return Flow::Skip;
	}

private:
	void WriteVariableDeclaration(const char* const type, const char* const rawName, const char* const description,
		const char* const defaultValue)
	{
		if (description != nullptr && description[0] != '\0')
		{
			m_output.NewLine();
			m_output << "// " << description;
		}

		const char lowerFirst = static_cast<char>(rawName[0]);

		m_output.NewLine();
		m_output << type << " m_" << lowerFirst << (rawName + 1);
		
		if (defaultValue != nullptr && defaultValue[0] != '\0')
		{
			m_output << "{ " << defaultValue << " };";
		}
		else
		{
			m_output << ";";
		}
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
			WriteStructMembersVisitor subVisitor{ m_schema, m_output, field.m_fieldID };
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
		case Asset::RecordSchemaFieldType::ImportedType:
		{
			outType += elementField->m_importedTypeData.m_importedTypeName.c_str();
			return true;
		}
		case Asset::RecordSchemaFieldType::Group:
		{
			// Write out an inner struct for the group with an uppercase first letter.
			const char* const groupName = elementField->m_fieldName.c_str();
			AMP_FATAL_ASSERT(groupName != nullptr && groupName[0] != '\0', "Group field [%u] is missing a name.",
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
}

void CodeGen::WriteMemberDeclarations(const Asset::RecordSchema& schema, CppStream& output)
{
	Internal_CodeGenUtil::WriteStructMembersVisitor writeVisitor{ schema, output, 0 };
	schema.Accept(writeVisitor, 0);
}
