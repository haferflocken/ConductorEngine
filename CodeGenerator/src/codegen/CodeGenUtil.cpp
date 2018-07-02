#include <codegen/CodeGenUtil.h>

#include <codegen/CppStream.h>

void CodeGen::WriteNamespaceDeclaration(
	const Collection::ArrayView<std::string>& namespaceNames,
	CodeGen::CppStream& output)
{
	output << "namespace " << namespaceNames[0].c_str();
	for (size_t i = 1; i < namespaceNames.Size(); ++i)
	{
		output << "::" << namespaceNames[i].c_str();
	}
}
