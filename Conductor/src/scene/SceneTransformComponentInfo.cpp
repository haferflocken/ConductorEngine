#include <scene/SceneTransformComponentInfo.h>

#include <mem/UniquePtr.h>

const Util::StringHash Scene::SceneTransformComponentInfo::sk_typeHash =
	Util::CalcHash(SceneTransformComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> Scene::SceneTransformComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<SceneTransformComponentInfo>();
}
