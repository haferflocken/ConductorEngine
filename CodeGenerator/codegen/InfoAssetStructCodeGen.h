#pragma once

#include <ostream>
#include <string>

namespace Asset { class RecordSchema; }

namespace CodeGen
{
void GenerateInfoInstanceStruct(const std::string* namespaceNames, const size_t numNamespaceNames,
	const Asset::RecordSchema& schema, std::ostream& outputStream);
void GenerateInfoInstanceSaveFunction(const std::string* namespaceNames, const size_t numNamespaceNames,
	const Asset::RecordSchema& schema, std::ostream& outputStream);
}
