#pragma once

#include <ecs/Component.h>
#include <ecs/ComponentInfo.h>

namespace Scene
{
class SceneSaveComponentInfo : public ECS::ComponentInfo
{
public:
	static constexpr const char* sk_typeName = "scene_save_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ComponentInfo> LoadFromJSON(
		Asset::AssetManager& assetManager, const JSON::JSONObject& jsonObject);

	SceneSaveComponentInfo() = default;
	virtual ~SceneSaveComponentInfo() = default;

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }
};

/**
 * Entities with a SceneSaveComponent are saved and loaded with the scene they are a part of.
 * SceneSaveComponent is a tag component and is therefore never instantiated.
 */
class SceneSaveComponent final : public ECS::Component
{
public:
	using Info = SceneSaveComponentInfo;
	SceneSaveComponent() = delete;
};
}

