#pragma once

#include <collection/Vector.h>
#include <ecs/ComponentInfo.h>
#include <mem/UniquePtr.h>

namespace ECS
{
class EntityInfo
{
public:
	static constexpr char* sk_typeName = "entity";
	static const Util::StringHash sk_typeHash;

	Util::StringHash m_nameHash;
	Collection::Vector<Mem::UniquePtr<ComponentInfo>> m_componentInfos;
};
}
