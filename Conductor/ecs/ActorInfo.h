#pragma once

#include <collection/Vector.h>
#include <ecs/ActorComponentInfo.h>
#include <mem/UniquePtr.h>

namespace ECS
{
class ActorInfo
{
public:
	static constexpr char* sk_typeName = "actor";
	static const Util::StringHash sk_typeHash;

	Collection::Vector<Mem::UniquePtr<ActorComponentInfo>> m_componentInfos;
};
}
