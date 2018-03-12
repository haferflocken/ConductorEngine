#include <islandgame/components/IslanderComponentInfo.h>

#include <mem/UniquePtr.h>

using namespace IslandGame;
using namespace IslandGame::Components;

const Util::StringHash IslanderComponentInfo::sk_typeHash = Util::CalcHash(IslanderComponentInfo::sk_typeName);

Mem::UniquePtr<Behave::ActorComponentInfo> IslanderComponentInfo::LoadFromJSON(const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<IslanderComponentInfo>();
}
