#pragma once

#include <collection/ArrayView.h>

#include <string>

namespace CodeGen
{
class CppStream;

void WriteNamespaceDeclaration(const Collection::ArrayView<std::string>& namespaceNames, CppStream& output);
}
