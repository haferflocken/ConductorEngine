#pragma once

#include <ecs/ComponentInfo.h>

namespace Behave { class BehaviourTreeManager; }
namespace JSON { class JSONObject; }
namespace Mem { template <typename T> class UniquePtr; }

namespace Behave
{
class BlackboardComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr char* sk_typeName = "blackboard_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ECS::ComponentInfo> LoadFromJSON(
		const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }
};
}