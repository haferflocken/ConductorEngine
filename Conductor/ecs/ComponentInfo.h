#pragma once

#include <mem/UniquePtr.h>
#include <util/StringHash.h>

namespace Asset { class AssetManager; }
namespace JSON { class JSONObject; }

namespace ECS
{
class ComponentInfo
{
public:
	virtual ~ComponentInfo() {}

	virtual const char* GetTypeName() const = 0;
	virtual Util::StringHash GetTypeHash() const = 0;
};
}
