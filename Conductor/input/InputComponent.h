#pragma once

#include <ecs/Component.h>
#include <ecs/ComponentInfo.h>

namespace Input
{
class InputComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr char* sk_typeName = "input_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ECS::ComponentInfo> LoadFromJSON(
		const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }
};

/**
 * An InputComponent makes an entity aware of user inputs.
 */
class InputComponent final : public ECS::Component
{
public:
	using Info = InputComponentInfo;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const InputComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	explicit InputComponent(const ECS::ComponentID id)
		: Component(id)
	{}
};
}
