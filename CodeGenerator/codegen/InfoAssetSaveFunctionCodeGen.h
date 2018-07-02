#pragma once

#include <collection/ArrayView.h>

#include <ostream>
#include <string>

namespace Asset { class RecordSchema; }

namespace CodeGen
{
void GenerateInfoInstanceSaveFunction(const Collection::ArrayView<std::string>& namespaceNames,
	const Asset::RecordSchema& schema, std::ostream& outputStream);
}
