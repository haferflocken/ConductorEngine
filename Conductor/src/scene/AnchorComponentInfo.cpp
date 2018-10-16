#include <scene/AnchorComponentInfo.h>

namespace Scene
{
const Util::StringHash AnchorComponentInfo::sk_typeHash = Util::CalcHash(AnchorComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> AnchorComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<AnchorComponentInfo>();
}
}
