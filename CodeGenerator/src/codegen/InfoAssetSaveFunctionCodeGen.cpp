#include <codegen/InfoAssetSaveFunctionCodeGen.h>

#include <asset/RecordSchema.h>
#include <codegen/CppStream.h>
#include <codegen/InfoAssetCodeGenUtil.h>

void CodeGen::GenerateInfoInstanceSaveFunction(
	const std::string* namespaceNames,
	const size_t numNamespaceNames,
	const Asset::RecordSchema& schema,
	std::ostream& outputStream)
{
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
