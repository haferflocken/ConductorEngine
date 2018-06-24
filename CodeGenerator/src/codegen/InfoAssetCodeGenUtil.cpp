#include <codegen/InfoAssetCodeGenUtil.h>

#include <codegen/CppStream.h>

void CodeGen::WriteNamespaceDeclaration(
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
