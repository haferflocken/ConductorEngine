#pragma once

#include <behave/ActorComponentInfo.h>
#include <collection/Vector.h>
#include <mem/UniquePtr.h>

namespace Behave
{
class ActorInfo
{
public:
	static constexpr char* sk_typeName = "actor";
	static const Util::StringHash sk_typeHash;

	Collection::Vector<Mem::UniquePtr<ActorComponentInfo>> m_componentInfos;
};
}
