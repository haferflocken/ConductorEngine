#pragma once

#include <behave/ActorComponentInfo.h>

namespace JSON { class JSONObject; }
namespace Mem { template <typename T> class UniquePtr; }

namespace Behave
{
class BehaviourTree;
class BehaviourTreeManager;

namespace Components
{
class BehaviourTreeComponentInfo final : public Behave::ActorComponentInfo
{
public:
	static constexpr char* sk_typeName = "behaviour_tree_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<Behave::ActorComponentInfo> LoadFromJSON(
		const BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }

	Collection::Vector<const BehaviourTree*> m_behaviourTrees{};
};
}
}
