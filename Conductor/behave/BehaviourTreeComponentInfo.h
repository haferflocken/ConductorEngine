#pragma once

#include <ecs/ComponentInfo.h>

namespace JSON { class JSONObject; }
namespace Mem { template <typename T> class UniquePtr; }

namespace Behave
{
class BehaviourTree;
class BehaviourTreeManager;

class BehaviourTreeComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr const char* sk_typeName = "behaviour_tree_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ECS::ComponentInfo> LoadFromJSON(
		const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }

	Collection::Vector<const Behave::BehaviourTree*> m_behaviourTrees{};
};
}
