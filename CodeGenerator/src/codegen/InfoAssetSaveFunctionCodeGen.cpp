#include <codegen/InfoAssetSaveFunctionCodeGen.h>

#include <asset/RecordSchema.h>
#include <asset/RecordSchemaVisitor.h>
#include <codegen/CodeGenUtil.h>
#include <codegen/CppStream.h>

namespace Internal_InfoAssetSaveFunction
{
class WriteInfoInstanceSaveFunctionVisitor : public Asset::RecordSchemaVisitor
{
	const Asset::RecordSchema& m_schema;
	Collection::Vector<std::string>& m_prefixNames;
	CodeGen::CppStream& m_output;
	const uint16_t m_rootFieldID;

public:
	WriteInfoInstanceSaveFunctionVisitor(
		const Asset::RecordSchema& schema,
		Collection::Vector<std::string>& prefixNames,
		CodeGen::CppStream& output,
		uint16_t rootFieldID)
		: m_schema(schema)
		, m_prefixNames(prefixNames)
		, m_output(output)
		, m_rootFieldID(rootFieldID)
	{}

	virtual ~WriteInfoInstanceSaveFunctionVisitor() {}

	virtual Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaBooleanData& fieldData) override
	{
		WriteFieldComment(field);
		WriteVariableSave("JSONBoolean", "m_boolean", field);
		return Flow::Visit;
	}

	virtual Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaFloatData& fieldData) override
	{
		WriteFieldComment(field);
		WriteVariableSave("JSONNumber", "m_number", field);
		return Flow::Visit;
	}

	virtual Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaIntegerData& fieldData) override
	{
		WriteFieldComment(field);
		WriteVariableSave("JSONNumber", "m_number", field);
		return Flow::Visit;
	}

	virtual Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaInstanceReferenceData& fieldData) override
	{
		WriteFieldComment(field);

		m_output.NewLine();
		m_output << '{';
		m_output.Indent([&]()
		{
			WriteVariableCreate("JSONString", "m_string", field.m_fieldName.c_str());
			m_output.NewLine();
			m_output << "value->m_hash = Util::CalcHash(value.m_string);";
			WriteVariableStore();
		});
		m_output.NewLine();
		m_output << '}';
		return Flow::Visit;
	}

	virtual Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaGroupData& fieldData) override
	{
		// If this group is the root, just visit its children.
		if (field.m_fieldID == m_rootFieldID)
		{
			return Flow::Visit;
		}

		WriteFieldComment(field);

		// Write out the group as an array of entry IDs.
		m_output.NewLine();
		m_output << '{';
		m_output.Indent([&]()
		{
			m_output.NewLine();
			m_output << "auto value = Mem::MakeUnique<JSON::JSONArray>();";
			for (size_t i = 0, iEnd = fieldData.m_memberFieldIDs.Size(); i < iEnd; ++i)
			{
				const auto& memberFieldID = fieldData.m_memberFieldIDs[i];

				m_output.NewLine();
				m_output << '{';
				m_output.Indent([&]()
				{
					m_output.NewLine();
					m_output << "auto element = Mem::MakeUnique<JSON::JSONNumber>();";
					m_output.NewLine();
					m_output << "element->m_number = entryID + " << std::to_string(i + 1).c_str() << ';';
					m_output.NewLine();
					m_output << "element->m_key = JSON::JSONKey(" << std::to_string(i).c_str() << "ui64);";
					m_output.NewLine();
					m_output << "value->Add(std::move(element));";
				});
				m_output.NewLine();
				m_output << '}';
			}
			WriteVariableStore();
		});
		m_output.NewLine();
		m_output << '}';

		// Write the entries the group references.
		if (field.m_fieldID != 0)
		{
			const char* const rawName = field.m_fieldName.c_str();
			const char lowerFirst = static_cast<char>(rawName[0]);
			std::string prefix = "m_";
			prefix += lowerFirst;
			prefix += (rawName + 1);
			m_prefixNames.Add(prefix);
		}

		WriteInfoInstanceSaveFunctionVisitor subVisitor{
			m_schema, m_prefixNames, m_output, field.m_fieldID };
		m_schema.Accept(subVisitor, field.m_fieldID);

		if (field.m_fieldID != 0)
		{
			m_prefixNames.RemoveLast();
		}

		// Do not visit the fields within the group because they were visited by the subVisitor.
		return Flow::Skip;
	}

	virtual Flow Visit(const Asset::RecordSchemaField& field, const Asset::RecordSchemaListData& fieldData) override
	{
		WriteFieldComment(field);

		// Write out the list as an array of entry IDs.
		m_output.NewLine();
		m_output << '{';
		m_output.Indent([&]()
		{
			m_output.NewLine();
			m_output << "auto value = Mem::MakeUnique<JSON::JSONArray>();";

			m_output.NewLine();
			m_output << "for (size_t i = 0, iEnd = ";
			if (field.m_fieldID != m_rootFieldID)
			{
				OutputVariableName(field.m_fieldName.c_str());
			}
			else
			{
				OutputPrefixes();
			}
			m_output << ".Size(); i < iEnd; ++i)";

			m_output.NewLine();
			m_output << '{';
			m_output.Indent([&]()
			{
				m_output.NewLine();
				m_output << "auto element = Mem::MakeUnique<JSON::JSONNumber>();";
				m_output.NewLine();
				m_output << "element->m_number = entryID + i + 1;";
				m_output.NewLine();
				m_output << "element->m_key = JSON::JSONKey(i);";
				m_output.NewLine();
				m_output << "value->Add(std::move(element));";
			});
			m_output.NewLine();
			m_output << '}';

			WriteVariableStore();
		});
		m_output.NewLine();
		m_output << '}';

		// Write out the entries the list references.
		const std::string fieldIDStr = std::to_string(field.m_fieldID);
		m_output.NewLine();
		m_output << "for (const auto& listElement" << fieldIDStr.c_str() << " : ";
		if (field.m_fieldID != m_rootFieldID)
		{
			OutputVariableName(field.m_fieldName.c_str());
		}
		else
		{
			OutputPrefixes();
		}
		m_output << ')';

		m_output.NewLine();
		m_output << '{';
		m_output.Indent([&]()
		{
			Collection::Vector<std::string> subPrefixNames;
			subPrefixNames.Emplace("listElement");
			subPrefixNames.Front() += fieldIDStr;

			WriteInfoInstanceSaveFunctionVisitor subVisitor{
				m_schema, subPrefixNames, m_output, fieldData.m_elementFieldID };
			m_schema.Accept(subVisitor, fieldData.m_elementFieldID);
		});
		m_output.NewLine();
		m_output << '}';

		return Flow::Skip;
	}

private:
	void WriteFieldComment(const Asset::RecordSchemaField& field)
	{
		m_output.NewLine();
		m_output << "// " << field.m_fieldName.c_str();
	}

	void OutputPrefixes()
	{
		for (auto iter = m_prefixNames.begin(), iterEnd = m_prefixNames.end() - 1; iter != iterEnd; ++iter)
		{
			m_output << iter->c_str() << '.';
		}
		m_output << m_prefixNames.Back().c_str();
	}

	void OutputVariableName(const char* const rawName)
	{
		for (const auto& prefixName : m_prefixNames)
		{
			m_output << prefixName.c_str() << '.';
		}
		const char lowerFirst = static_cast<char>(rawName[0]);
		m_output << "m_" << lowerFirst << (rawName + 1);
	}

	void WriteVariableCreate(const char* const jsonType, const char* const jsonValueMember, const char* const rawName)
	{
		m_output.NewLine();
		m_output << "auto value = Mem::MakeUnique<JSON::" << jsonType << ">();";
		m_output.NewLine();
		m_output << "value->" << jsonValueMember << " = ";
		OutputVariableName(rawName);
		m_output << ';';
	}

	void WriteVariableStore()
	{
		m_output.NewLine();
		m_output << "JSON::JSONObject::Entry& entry = savedInstance.Emplace(std::to_string(entryID), std::move(value));";
		m_output.NewLine();
		m_output << "entry.second->m_key = JSON::JSONKey(entry.first.data());";
		m_output.NewLine();
		m_output << "++entryID;";
	}

	void WriteVariableSave(
		const char* const jsonType,
		const char* const jsonValueMember,
		const Asset::RecordSchemaField& field)
	{
		m_output.NewLine();
		m_output << '{';
		m_output.Indent([&]()
		{
			WriteVariableCreate(jsonType, jsonValueMember, field.m_fieldName.c_str());
			WriteVariableStore();
		});
		m_output.NewLine();
		m_output << '}';
	}
};
}

void CodeGen::GenerateInfoInstanceSaveFunction(
	const Collection::ArrayView<std::string>& namespaceNames,
	const Asset::RecordSchema& schema,
	std::ostream& outputStream)
{
	using namespace Internal_InfoAssetSaveFunction;

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
	WriteNamespaceDeclaration(namespaceNames, output);
	output << "\n{";

	// Write out the save function.
	output.NewLine();
	output << "JSON::JSONObject SaveInfoInstance(const ";
	output.AppendCapitalized(name);
	output << "& infoInstance)\n{";

	output.Indent([&]()
	{
		output.NewLine();
		output << "JSON::JSONObject savedInstance;";
		output.NewLine();
		output << "size_t entryID = 0;";

		Collection::Vector<std::string> prefixNames;
		prefixNames.Emplace("infoInstance");

		WriteInfoInstanceSaveFunctionVisitor writeVisitor{ schema, prefixNames, output, UINT16_MAX };
		schema.Accept(writeVisitor, 0);

		output.NewLine();
		output << "return savedInstance;";
	});

	output.NewLine();
	output << "}\n";

	// Close the namespaces brace.
	output << "}\n";
}
