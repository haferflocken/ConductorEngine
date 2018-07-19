#include <ecs/components/BlackboardComponentInfo.h>

#include <mem/UniquePtr.h>

const Util::StringHash ECS::Components::BlackboardComponentInfo::sk_typeHash =
	Util::CalcHash(BlackboardComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> ECS::Components::BlackboardComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<BlackboardComponentInfo>();
}
