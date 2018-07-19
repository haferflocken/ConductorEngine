#pragma once

#include <ecs/ActorComponent.h>

#include <util/StringHash.h>

namespace ECS
{
class ActorComponentInfo;
class ActorComponentVector;
class ActorManager;
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
class IslanderComponent final : public ECS::ActorComponent
{
public:
	using Info = IslanderComponentInfo;

	static bool TryCreateFromInfo(const IslanderComponentInfo& componentInfo,
		const ECS::ActorComponentID reservedID, ECS::ActorComponentVector& destination);
	
	explicit IslanderComponent(const ECS::ActorComponentID id)
		: ActorComponent(id)
		, m_hunger(0)
		, m_tiredness(0)
	{}

	int32_t m_hunger;
	int32_t m_tiredness;
};
}
}
