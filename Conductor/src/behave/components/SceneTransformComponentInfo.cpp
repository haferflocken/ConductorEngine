#include <behave/components/SceneTransformComponentInfo.h>

#include <mem/UniquePtr.h>

const Util::StringHash Behave::Components::SceneTransformComponentInfo::sk_typeHash =
	Util::CalcHash(SceneTransformComponentInfo::sk_typeName);

Mem::UniquePtr<Behave::ActorComponentInfo> Behave::Components::SceneTransformComponentInfo::LoadFromJSON(
	const BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<SceneTransformComponentInfo>();
}
