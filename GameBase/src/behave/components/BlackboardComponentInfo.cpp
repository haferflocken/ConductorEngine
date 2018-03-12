#include <behave/components/BlackboardComponentInfo.h>

#include <mem/UniquePtr.h>

using namespace Behave;
using namespace Behave::Components;

const Util::StringHash BlackboardComponentInfo::sk_typeHash =
	Util::CalcHash(BlackboardComponentInfo::sk_typeName);

Mem::UniquePtr<Behave::ActorComponentInfo> BlackboardComponentInfo::LoadFromJSON(
	const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<BlackboardComponentInfo>();
}
