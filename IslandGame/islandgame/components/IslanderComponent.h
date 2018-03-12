#pragma once

#include <behave/ActorComponent.h>

#include <util/StringHash.h>

namespace Behave
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
class IslanderComponent final : public Behave::ActorComponent
{
public:
	using Info = IslanderComponentInfo;

	static bool TryCreateFromInfo(const IslanderComponentInfo& componentInfo,
		const Behave::ActorComponentID reservedID, Behave::ActorComponentVector& destination);
	
	explicit IslanderComponent(const Behave::ActorComponentID id)
		: ActorComponent(id)
		, m_hunger(0)
		, m_tiredness(0)
	{}

	int32_t m_hunger;
	int32_t m_tiredness;
};
}
}
