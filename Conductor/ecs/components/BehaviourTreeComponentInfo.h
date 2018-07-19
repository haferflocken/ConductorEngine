#pragma once

#include <ecs/ComponentInfo.h>

namespace Behave
{
class BehaviourTree;
class BehaviourTreeManager;
}

namespace JSON { class JSONObject; }
namespace Mem { template <typename T> class UniquePtr; }

namespace ECS::Components
{
class BehaviourTreeComponentInfo final : public ComponentInfo
{
public:
	static constexpr char* sk_typeName = "behaviour_tree_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ComponentInfo> LoadFromJSON(
		const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }

	Collection::Vector<const Behave::BehaviourTree*> m_behaviourTrees{};
};
}
