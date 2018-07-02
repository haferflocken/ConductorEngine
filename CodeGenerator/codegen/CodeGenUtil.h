#pragma once

#include <string>

namespace CodeGen
{
class CppStream;

void WriteNamespaceDeclaration(const std::string* namespaceNames, const size_t numNamespaceNames, CppStream& output);
}
