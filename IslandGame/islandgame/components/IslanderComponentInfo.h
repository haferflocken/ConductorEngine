#pragma once

#include <behave/ActorComponentInfo.h>

namespace Behave { class BehaviourTreeManager; }
namespace JSON { class JSONObject; }
namespace Mem { template <typename T> class UniquePtr; }

namespace IslandGame
{
namespace Components
{
class IslanderComponentInfo final : public Behave::ActorComponentInfo
{
public:
	static constexpr char* sk_typeName = "islander_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<Behave::ActorComponentInfo> LoadFromJSON(
		const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }
};
}
}
