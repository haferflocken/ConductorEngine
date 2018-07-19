#pragma once

#include <collection/VectorMap.h>
#include <ecs/ActorInfo.h>
#include <file/Path.h>
#include <util/StringHash.h>

namespace Behave { class BehaviourTreeManager; }

namespace JSON { class JSONObject; }

namespace ECS
{
class ActorComponentInfoFactory;

/**
 * Loads and owns actor info.
 */
class ActorInfoManager
{
public:
	ActorInfoManager(const ActorComponentInfoFactory& componentInfoFactory,
		const Behave::BehaviourTreeManager& treeManager)
		: m_actorComponentInfoFactory(componentInfoFactory)
		, m_behaviourTreeManager(treeManager)
		, m_actorInfos()
	{}

	void LoadActorInfosInDirectory(const File::Path& directory);
	
	const ActorInfo* FindActorInfo(const Util::StringHash actorInfoNameHash) const;

private:
	const ActorComponentInfoFactory& m_actorComponentInfoFactory;
	const Behave::BehaviourTreeManager& m_behaviourTreeManager;
	Collection::VectorMap<Util::StringHash, ActorInfo> m_actorInfos;
};
}
