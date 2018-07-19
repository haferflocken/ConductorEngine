#include <ecs/components/SceneTransformComponentInfo.h>

#include <mem/UniquePtr.h>

const Util::StringHash ECS::Components::SceneTransformComponentInfo::sk_typeHash =
	Util::CalcHash(SceneTransformComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> ECS::Components::SceneTransformComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<SceneTransformComponentInfo>();
}
