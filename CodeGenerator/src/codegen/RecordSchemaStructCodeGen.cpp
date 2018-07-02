#include <codegen/RecordSchemaStructCodeGen.h>

#include <asset/RecordSchema.h>
#include <asset/RecordSchemaVisitor.h>
#include <codegen/CodeGenUtil.h>
#include <codegen/CppStream.h>
#include <dev/Dev.h>

void CodeGen::GenerateInfoAssetStructFromRecordSchema(
	const Collection::ArrayView<std::string> namespaceNames,
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
	output << "#include <collection/Vector.h>\n";
	output << "#include <cstdint>\n";
	output << "#include <string>\n";

	// Write out the namespaces.
	output.NewLine();
	WriteNamespaceDeclaration(namespaceNames, output);
	output << "\n{";
	
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
	const char* const name = schema.GetName().c_str();
	if (name == nullptr || name[0] == '\0')
	{
		return;
	}

	CppStream output{ outputStream };
	output << "// GENERATED CODE\n";

	// Write out the required includes.
	output << "#include <behave/ActorComponent.h>\n";
	output << "#include <collection/Vector.h>\n";
	output << "#include <cstdint>\n";
	output << "#include <string>\n";

	// Write out forward declarations.
	output.NewLine();
	output << "namespace Behave { class ActorComponentVector; }\n";

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

	// Write out the struct.
	output.NewLine();
	output << "class ";
	output.AppendCapitalized(name);
	output << " final : public Behave::ActorComponent\n{";
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
		output << "\tconst ActorComponentID reservedID, ActorComponentVector& destination);";
		output.NewLine();

		// Main constructor.
		output.NewLine();
		output << "explicit ";
		output.AppendCapitalized(name);
		output << "(const Behave::ActorComponentID id)";
		output.NewLine();
		output << "\t: Behave::ActorComponent(id)";
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
