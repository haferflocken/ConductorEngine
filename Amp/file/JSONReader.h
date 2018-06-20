#pragma once

#include <file/Path.h>

namespace JSON { class JSONValue; }
namespace Mem { template <typename T> class UniquePtr; }

namespace File
{
Mem::UniquePtr<JSON::JSONValue> ReadJSONFile(const Path& path);
}
