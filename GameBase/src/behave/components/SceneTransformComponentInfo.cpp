#include <behave/components/SceneTransformComponentInfo.h>

#include <mem/UniquePtr.h>

using namespace Behave;
using namespace Behave::Components;

const Util::StringHash SceneTransformComponentInfo::sk_typeHash =
	Util::CalcHash(SceneTransformComponentInfo::sk_typeName);

Mem::UniquePtr<Behave::ActorComponentInfo> SceneTransformComponentInfo::LoadFromJSON(
	const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<SceneTransformComponentInfo>();
}
