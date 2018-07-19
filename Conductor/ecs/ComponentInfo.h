#pragma once

#include <util/StringHash.h>

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
