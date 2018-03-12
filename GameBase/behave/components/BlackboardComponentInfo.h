#pragma once

#include <behave/ActorComponentInfo.h>

namespace JSON { class JSONObject; }
namespace Mem { template <typename T> class UniquePtr; }

namespace Behave
{
namespace Components
{
class BlackboardComponentInfo final : public Behave::ActorComponentInfo
{
public:
	static constexpr char* sk_typeName = "blackboard_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<Behave::ActorComponentInfo> LoadFromJSON(const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }
};
}
}
