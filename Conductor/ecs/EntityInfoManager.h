#pragma once

#include <collection/VectorMap.h>
#include <ecs/EntityInfo.h>
#include <file/Path.h>
#include <util/StringHash.h>

namespace Behave { class BehaviourTreeManager; }
namespace JSON { class JSONObject; }

namespace ECS
{
class ComponentInfoFactory;

/**
 * Loads and owns entity info.
 */
class EntityInfoManager
{
public:
	EntityInfoManager(const ComponentInfoFactory& componentInfoFactory,
		const Behave::BehaviourTreeManager& treeManager)
		: m_componentInfoFactory(componentInfoFactory)
		, m_behaviourTreeManager(treeManager)
		, m_entityInfos()
	{}

	void LoadEntityInfosInDirectory(const File::Path& directory);
	
	const EntityInfo* FindEntityInfo(const Util::StringHash entityInfoNameHash) const;

private:
	const ComponentInfoFactory& m_componentInfoFactory;
	const Behave::BehaviourTreeManager& m_behaviourTreeManager;
	Collection::VectorMap<Util::StringHash, EntityInfo> m_entityInfos;
};
}
