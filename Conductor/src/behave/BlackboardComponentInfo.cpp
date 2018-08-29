#include <behave/BlackboardComponentInfo.h>

#include <mem/UniquePtr.h>

const Util::StringHash Behave::BlackboardComponentInfo::sk_typeHash =
	Util::CalcHash(BlackboardComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> Behave::BlackboardComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<BlackboardComponentInfo>();
}