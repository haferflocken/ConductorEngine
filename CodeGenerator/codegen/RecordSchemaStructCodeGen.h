#pragma once

#include <collection/ArrayView.h>

#include <ostream>
#include <string>

namespace Asset { class RecordSchema; }

namespace CodeGen
{
struct StructGenParams
{
	const Collection::ArrayView<std::string> namespaceNames;
	std::string parentNames;
	const Asset::RecordSchema& schema;
};

void GenerateStructFromRecordSchema(const StructGenParams& params, std::ostream& outputStream);
}
