#pragma once

#include <ecs/Component.h>

namespace Scene
{
class AnchorComponentInfo;

/**
 * Entities with an AnchorComponent cause the scene to load around them.
 */
class AnchorComponent final : public ECS::Component
{
public:
	using Info = AnchorComponentInfo;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const AnchorComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);

	explicit AnchorComponent(const ECS::ComponentID id)
		: ECS::Component(id)
	{}

	virtual ~AnchorComponent() {}
};
}
