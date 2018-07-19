#pragma once

#include <util/StringHash.h>

namespace ECS
{
class ActorComponentInfo
{
public:
	virtual ~ActorComponentInfo() {}

	virtual const char* GetTypeName() const = 0;
	virtual Util::StringHash GetTypeHash() const = 0;
};
}
