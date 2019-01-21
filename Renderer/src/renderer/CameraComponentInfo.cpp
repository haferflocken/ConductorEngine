#include <renderer/CameraComponentInfo.h>

namespace Renderer
{
const Util::StringHash CameraComponentInfo::sk_typeHash = Util::CalcHash(CameraComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> CameraComponentInfo::LoadFromJSON(
	Asset::AssetManager& assetManager, const JSON::JSONObject& jsonObject)
{
	return Mem::MakeUnique<CameraComponentInfo>();
}
}
