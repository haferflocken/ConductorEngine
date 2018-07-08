#pragma once

#include <collection/ArrayView.h>

#include <ostream>
#include <string>

namespace Asset { class RecordSchema; }

namespace CodeGen
{
/**
 * Generates a C++ function which takes an Info Asset's C++ instance and serializes it as JSON.
 */
void GenerateInfoInstanceSaveFunction(const Collection::ArrayView<std::string>& namespaceNames,
	const Asset::RecordSchema& schema, std::ostream& outputStream);
}
