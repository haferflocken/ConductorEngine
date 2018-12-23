#pragma once

#include <ecs/Component.h>
#include <ecs/ComponentInfo.h>

#include <math/Matrix4x4.h>

namespace Condui
{
class UITransformComponentInfo final : public ECS::ComponentInfo
{
public:
	static constexpr const char* sk_typeName = "ui_transform_component";
	static const Util::StringHash sk_typeHash;

	static Mem::UniquePtr<ComponentInfo> LoadFromJSON(
		const Behave::BehaviourTreeManager& behaviourTreeManager, const JSON::JSONObject& jsonObject);

	virtual const char* GetTypeName() const override { return sk_typeName; }
	virtual Util::StringHash GetTypeHash() const override { return sk_typeHash; }
};

/**
 * A UITransformComponent gives an entity a position, scale, and orientation within the UI.
 */
class UITransformComponent final : public ECS::Component
{
public:
	using Info = UITransformComponentInfo;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const UITransformComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	explicit UITransformComponent(const ECS::ComponentID id)
		: ECS::Component(id)
	{}

	virtual ~UITransformComponent() {}

	Math::Matrix4x4 m_uiTransform{};
	Math::Matrix4x4 m_transformFromParent{};
};
}
