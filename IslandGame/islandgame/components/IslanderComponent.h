#pragma once

#include <ecs/Component.h>

#include <util/StringHash.h>

namespace ECS
{
class ComponentInfo;
class ComponentVector;
class EntityManager;
}

namespace IslandGame
{
namespace Components
{
class IslanderComponentInfo;

/**
 * An islander lives on an island and has two priorities:
 * - To survive, by staying fed and getting sleep
 * - To perform its job
 * Islander behaviour is implemented by behaviour trees.
 */
class IslanderComponent final : public ECS::Component
{
public:
	using Info = IslanderComponentInfo;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager, const IslanderComponentInfo& componentInfo,
		const ECS::ComponentID reservedID, ECS::ComponentVector& destination);
	
	explicit IslanderComponent(const ECS::ComponentID id)
		: Component(id)
		, m_hunger(0)
		, m_tiredness(0)
	{}

	int32_t m_hunger;
	int32_t m_tiredness;
};
}
}
