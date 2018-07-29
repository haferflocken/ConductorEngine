#include <codegen/RecordSchemaStructCodeGen.h>

#include <asset/RecordSchema.h>
#include <asset/RecordSchemaVisitor.h>
#include <codegen/CodeGenUtil.h>
#include <codegen/CppStream.h>
#include <dev/Dev.h>

namespace Internal_RecordSchemaStructCodeGen
{
void WriteRootDescription(const Asset::RecordSchema& schema, CodeGen::CppStream& output)
{
	const Asset::RecordSchemaField* const rootGroup = schema.FindField(0);
	if (rootGroup != nullptr && !rootGroup->m_fieldDescription.empty())
	{
		output.NewLine();
		output << "/**";
		output.NewLine();
		output << " * " << rootGroup->m_fieldDescription.c_str();
		output.NewLine();
		output << " */";
	}
}
}

void CodeGen::GenerateInfoAssetStructFromRecordSchema(
	const Collection::ArrayView<std::string> namespaceNames,
	const Asset::RecordSchema& schema,
	std::ostream& outputStream)
{
	using namespace Internal_RecordSchemaStructCodeGen;

	const char* const name = schema.GetName().c_str();
	if (name == nullptr || name[0] == '\0')
	{
		return;
	}

	CppStream output{ outputStream };
	output << "// GENERATED CODE\n";
	output << "#pragma once\n\n";

	// Write out the required includes.
	output << "#include <collection/Vector.h>\n";
	output << "#include <cstdint>\n";
	output << "#include <string>\n";

	// Write out the namespaces.
	output.NewLine();
	WriteNamespaceDeclaration(namespaceNames, output);
	output << "\n{";

	// Write out the root group's description as a struct-level description.
	WriteRootDescription(schema, output);

	// Write out the struct.
	output.NewLine();
	output << "struct ";
	output.AppendCapitalized(name);
	output << " final\n{";

	output.Indent([&]()
	{
		WriteMemberDeclarations(schema, output);
	});

	output.NewLine();
	output << "};\n";

	// Close the namespaces brace.
	output << "}\n";
}
