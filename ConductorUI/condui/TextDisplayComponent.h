#pragma once

#include <ecs/Component.h>
#include <ecs/ComponentInfo.h>

namespace Condui
{
class TextDisplayComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr const char* sk_typeName = "text_display_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ComponentInfo> LoadFromJSON(
		const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const { return sk_typeHash; }
};

/**
 * A TextDisplayComponent makes an entity appear in the UI as a text display.
 */
class TextDisplayComponent final : public ECS::Component
{
public:
	using Info = TextDisplayComponentInfo;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const TextDisplayComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	explicit TextDisplayComponent(const ECS::ComponentID id)
		: ECS::Component(id)
	{}

	virtual ~TextDisplayComponent() {}

	std::string m_string;
};
}
