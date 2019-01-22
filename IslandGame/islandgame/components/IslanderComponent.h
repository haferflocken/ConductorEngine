#pragma once

#include <ecs/Component.h>

namespace IslandGame
{
namespace Components
{
/**
 * An islander lives on an island and has two priorities:
 * - To survive, by staying fed and getting sleep
 * - To perform its job
 * Islander behaviour is implemented by behaviour trees.
 */
class IslanderComponent final : public ECS::Component
{
public:
	static constexpr char* k_typeName = "islander_component";
	static const Util::StringHash k_typeHash;

	static bool TryCreateFromInfo(Asset::AssetManager& assetManager,
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
