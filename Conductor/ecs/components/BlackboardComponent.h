#pragma once

#include <behave/Blackboard.h>
#include <ecs/Component.h>

namespace ECS
{
class ComponentVector;

namespace Components
{
class BlackboardComponentInfo;

/**
 * A BlackboardComponent contains a blackboard: a data-driven key/value store.
 */
class BlackboardComponent final : public Component
{
public:
	using Info = BlackboardComponentInfo;

	static bool TryCreateFromInfo(const BlackboardComponentInfo& componentInfo, const ComponentID reservedID,
		ComponentVector& destination);

	explicit BlackboardComponent(const ComponentID id)
		: Component(id)
		, m_blackboard()
	{}

	virtual ~BlackboardComponent() {}

	Behave::Blackboard m_blackboard;
};
}
}


