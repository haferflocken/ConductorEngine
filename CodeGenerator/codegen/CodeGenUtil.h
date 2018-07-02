#pragma once

#include <collection/ArrayView.h>

#include <string>

namespace Asset { class RecordSchema; }

namespace CodeGen
{
class CppStream;

void WriteNamespaceDeclaration(const Collection::ArrayView<std::string>& namespaceNames, CppStream& output);
void WriteMemberDeclarations(const Asset::RecordSchema& schema, CppStream& output);
}
