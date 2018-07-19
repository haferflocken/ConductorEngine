#include <islandgame/components/IslanderComponentInfo.h>

#include <mem/UniquePtr.h>

const Util::StringHash IslandGame::Components::IslanderComponentInfo::sk_typeHash =
	Util::CalcHash(IslanderComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ActorComponentInfo> IslandGame::Components::IslanderComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<IslanderComponentInfo>();
}
