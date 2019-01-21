#pragma once

#include <ecs/ComponentInfo.h>
#include <file/Path.h>

namespace Mesh
{
class MeshComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr const char* sk_typeName = "mesh_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ECS::ComponentInfo> LoadFromJSON(
		Asset::AssetManager& assetManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }

	File::Path m_meshFilePath;
};
}
