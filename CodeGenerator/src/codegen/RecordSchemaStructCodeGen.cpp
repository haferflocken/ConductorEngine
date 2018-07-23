#include <codegen/RecordSchemaStructCodeGen.h>

#include <asset/RecordSchema.h>
#include <asset/RecordSchemaVisitor.h>
#include <codegen/CodeGenUtil.h>
#include <codegen/CppStream.h>
#include <dev/Dev.h>

namespace Internal_RecordSchemaStructCodeGen
{
void WriteImportedTypeIncludes(const Asset::RecordSchema& schema, CodeGen::CppStream& output)
{
	const auto& importedTypes = schema.GetImportedTypes();
	Collection::VectorMap<std::string, char> includeSet;
	for (const auto& entry : importedTypes)
	{
		includeSet[entry.second] = '\0';
	}

	for (const auto& entry : includeSet)
	{
		const auto& includeString = entry.first;
		if (!includeString.empty())
		{
			output.NewLine();
			output << "#include <" << includeString.c_str() << '>';
		}
	}
}

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
	WriteImportedTypeIncludes(schema, output);
	output.NewLine();
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

void CodeGen::GenerateComponentClassFromRecordSchema(
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
	output << "#include <ecs/Component.h>";
	WriteImportedTypeIncludes(schema, output);
	output.NewLine();
	output << "#include <collection/Vector.h>\n";
	output << "#include <cstdint>\n";
	output << "#include <string>\n";

	// Write out forward declarations.
	output.NewLine();
	output << "namespace ECS { class ComponentVector; }\n";

	// Write out the namespaces.
	output.NewLine();
	WriteNamespaceDeclaration(namespaceNames, output);
	output << "\n{";

	// Write out inner forward declarations.
	output.NewLine();
	output << "class ";
	output.AppendCapitalized(name);
	output << "Info;";
	output.NewLine();

	// Write out the root group's description as a class-level description.
	WriteRootDescription(schema, output);

	// Write out the class.
	output.NewLine();
	output << "class ";
	output.AppendCapitalized(name);
	output << " final : public ECS::Component\n{";
	output.NewLine();
	output << "public:";

	output.Indent([&]()
	{
		output.NewLine();
		output << "using Info = ";
		output.AppendCapitalized(name);
		output << "Info;";
		output.NewLine();

		// Factory function.
		output << "static bool TryCreateFromInfo(const Info& componentInfo, ";
		output.NewLine();
		output << "\tconst ECS::ComponentID reservedID, ECS::ComponentVector& destination);";
		output.NewLine();

		// Main constructor.
		output.NewLine();
		output << "explicit ";
		output.AppendCapitalized(name);
		output << "(const ECS::ComponentID id)";
		output.NewLine();
		output << "\t: ECS::Component(id)";
		output.NewLine();
		output << "{}";
		output.NewLine();

		// Copy and move constructors and operators.
		output.NewLine();
		output.AppendCapitalized(name);
		output << "(const ";
		output.AppendCapitalized(name);
		output << "&) = delete;";
		output.NewLine();
		output.AppendCapitalized(name);
		output << "& operator=(const ";
		output.AppendCapitalized(name);
		output << "&) = delete;";
		output.NewLine();

		output.NewLine();
		output.AppendCapitalized(name);
		output << "(";
		output.AppendCapitalized(name);
		output << "&&) = default;";
		output.NewLine();
		output.AppendCapitalized(name);
		output << "& operator=(";
		output.AppendCapitalized(name);
		output << "&&) = default;";
		output.NewLine();

		// Destructor.
		output.NewLine();
		output << "virtual ~";
		output.AppendCapitalized(name);
		output << "() {}";
		output.NewLine();

		// Members.
		WriteMemberDeclarations(schema, output);
	});

	output.NewLine();
	output << "};\n";

	// Close the namespaces brace.
	output << "}\n";
}
