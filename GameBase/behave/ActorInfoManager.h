#pragma once

#include <behave/ActorInfo.h>
#include <collection/VectorMap.h>
#include <file/Path.h>
#include <util/StringHash.h>

namespace JSON
{
class JSONObject;
}

namespace Behave
{
class ActorComponentInfoFactory;
class BehaviourTreeManager;

/**
 * Loads and owns actor info.
 */
class ActorInfoManager
{
public:
	ActorInfoManager(const ActorComponentInfoFactory& componentInfoFactory, const BehaviourTreeManager& treeManager)
		: m_actorComponentInfoFactory(componentInfoFactory)
		, m_behaviourTreeManager(treeManager)
		, m_actorInfos()
	{}

	void LoadActorInfosInDirectory(const File::Path& directory);
	
	const ActorInfo* FindActorInfo(const Util::StringHash actorInfoNameHash) const;

private:
	const ActorComponentInfoFactory& m_actorComponentInfoFactory;
	const BehaviourTreeManager& m_behaviourTreeManager;
	Collection::VectorMap<Util::StringHash, ActorInfo> m_actorInfos;
};
}