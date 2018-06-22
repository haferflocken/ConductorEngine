#pragma once

#include <string>

namespace Asset { class RecordSchema; }

namespace CodeGen
{
std::string GenerateInfoInstanceStruct(const Asset::RecordSchema& schema);
std::string GenerateInfoInstanceSaveFunction(const Asset::RecordSchema& schema);
std::string GenerateInfoInstanceLoadFunction(const Asset::RecordSchema& schema);
}
