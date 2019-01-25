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
	static constexpr ECS::ComponentBindingType k_bindingType = ECS::ComponentBindingType::MemoryImaged;
	static constexpr char* k_typeName = "islander_component";
	static const ECS::ComponentType k_type;

	explicit IslanderComponent(const ECS::ComponentID id)
		: Component(id)
	{}

	int32_t m_hunger{ 0 };
	int32_t m_tiredness{ 0 };
};
}
}
