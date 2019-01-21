#include <mesh/MeshComponentInfo.h>

#include <json/JSONTypes.h>

namespace Mesh
{
namespace Internal_MeshComponentInfo
{
const Util::StringHash k_meshFilePathHash = Util::CalcHash("mesh_file_path");
}

const Util::StringHash MeshComponentInfo::sk_typeHash = Util::CalcHash(MeshComponentInfo::sk_typeName);

Mem::UniquePtr<ECS::ComponentInfo> MeshComponentInfo::LoadFromJSON(
	Asset::AssetManager& assetManager, const JSON::JSONObject& jsonObject)
{
	const JSON::JSONString* const meshFilePath = jsonObject.FindString(Internal_MeshComponentInfo::k_meshFilePathHash);
	if (meshFilePath == nullptr)
	{
		return nullptr;
	}

	auto info = Mem::MakeUnique<MeshComponentInfo>();
	info->m_meshFilePath = meshFilePath->m_string;
	return info;
}
}
