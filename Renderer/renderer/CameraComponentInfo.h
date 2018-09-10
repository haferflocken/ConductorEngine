#pragma once

#include <ecs/ComponentInfo.h>

namespace Renderer
{
class CameraComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr const char* sk_typeName = "camera_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ECS::ComponentInfo> LoadFromJSON(
		const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }
};
}
