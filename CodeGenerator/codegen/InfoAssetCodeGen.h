#pragma once

#include <string>

namespace Asset { class RecordSchema; }

namespace CodeGen
{
std::string GenerateInfoInstanceStruct(const std::string* namespaceNames, const size_t numNamespaceNames,
	const Asset::RecordSchema& schema);
std::string GenerateInfoInstanceSaveFunction(const std::string* namespaceNames, const size_t numNamespaceNames,
	const Asset::RecordSchema& schema);
std::string GenerateInfoInstanceLoadFunction(const std::string* namespaceNames, const size_t numNamespaceNames,
	const Asset::RecordSchema& schema);
}
