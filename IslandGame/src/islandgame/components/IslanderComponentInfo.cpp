#include <islandgame/components/IslanderComponentInfo.h>

#include <mem/UniquePtr.h>

const Util::StringHash IslandGame::Components::IslanderComponentInfo::sk_typeHash =
	Util::CalcHash(IslanderComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> IslandGame::Components::IslanderComponentInfo::LoadFromJSON(
	Asset::AssetManager& assetManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<IslanderComponentInfo>();
}
