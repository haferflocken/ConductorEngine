#include <renderer/MeshComponentInfo.h>

namespace Renderer
{
const Util::StringHash MeshComponentInfo::sk_typeHash = Util::CalcHash(MeshComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> MeshComponentInfo::LoadFromJSON(
	const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<MeshComponentInfo>();
}
}
