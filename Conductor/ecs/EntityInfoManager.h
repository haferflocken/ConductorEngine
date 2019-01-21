#pragma once

#include <collection/VectorMap.h>
#include <ecs/EntityInfo.h>
#include <file/Path.h>
#include <util/StringHash.h>

namespace Asset { class AssetManager; }
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
	EntityInfoManager(Asset::AssetManager& assetManager, const ComponentInfoFactory& componentInfoFactory)
		: m_assetManager(assetManager)
		, m_componentInfoFactory(componentInfoFactory)
		, m_entityInfos()
	{}

	void LoadEntityInfosInDirectory(const File::Path& directory);

	void RegisterEntityInfo(const Util::StringHash nameHash, EntityInfo&& info);
	
	const EntityInfo* FindEntityInfo(const Util::StringHash entityInfoNameHash) const;

private:
	Asset::AssetManager& m_assetManager;
	const ComponentInfoFactory& m_componentInfoFactory;

	Collection::VectorMap<Util::StringHash, EntityInfo> m_entityInfos;
};
}
