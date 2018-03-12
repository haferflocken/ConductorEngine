#pragma once

#include <behave/ActorComponentInfo.h>
#include <collection/Vector.h>
#include <mem/UniquePtr.h>

namespace Behave
{
class BehaviourTree;

class ActorInfo
{
public:
	Collection::Vector<Mem::UniquePtr<ActorComponentInfo>> m_componentInfos;
	Collection::Vector<const BehaviourTree*> m_behaviourTrees;
};
}
