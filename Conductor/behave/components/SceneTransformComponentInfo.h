#pragma once

#include <behave/ActorComponentInfo.h>

namespace JSON { class JSONObject; }
namespace Mem { template <typename T> class UniquePtr; }

namespace Behave
{
class BehaviourTreeManager;

namespace Components
{
class SceneTransformComponentInfo final : public Behave::ActorComponentInfo
{
public:
	static constexpr char* sk_typeName = "scene_transform_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<Behave::ActorComponentInfo> LoadFromJSON(
		const BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }
};
}
}