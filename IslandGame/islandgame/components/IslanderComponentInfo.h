#pragma once

#include <ecs/ComponentInfo.h>

namespace IslandGame
{
namespace Components
{
class IslanderComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr char* sk_typeName = "islander_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ECS::ComponentInfo> LoadFromJSON(
		Asset::AssetManager& assetManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }
};
}
}
