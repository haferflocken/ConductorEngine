#include <scene/SceneSaveComponent.h>

#include <mem/UniquePtr.h>

const Util::StringHash Scene::SceneSaveComponentInfo::sk_typeHash = Util::CalcHash(SceneSaveComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> Scene::SceneSaveComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager,
	const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<SceneSaveComponentInfo>();
}
