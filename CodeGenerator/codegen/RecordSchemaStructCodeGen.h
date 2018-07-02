#pragma once

#include <ostream>
#include <string>

namespace Asset { class RecordSchema; }

namespace CodeGen
{
struct StructGenParams
{
	const std::string* namespaceNames;
	size_t numNamespaceNames;
	const Asset::RecordSchema& schema;
};

void GenerateStructFromRecordSchema(const StructGenParams& params, std::ostream& outputStream);
}
