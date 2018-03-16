#include <behave/components/BlackboardComponentInfo.h>

#include <mem/UniquePtr.h>

const Util::StringHash Behave::Components::BlackboardComponentInfo::sk_typeHash =
	Util::CalcHash(BlackboardComponentInfo::sk_typeName);

Mem::UniquePtr<Behave::ActorComponentInfo> Behave::Components::BlackboardComponentInfo::LoadFromJSON(
	const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<BlackboardComponentInfo>();
}
