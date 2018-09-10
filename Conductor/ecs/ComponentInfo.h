#pragma once

#include <mem/UniquePtr.h>
#include <util/StringHash.h>

namespace Behave { class BehaviourTreeManager; }
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
