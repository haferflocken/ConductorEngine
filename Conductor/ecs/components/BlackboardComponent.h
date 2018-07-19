#pragma once

#include <behave/Blackboard.h>
#include <ecs/ActorComponent.h>

namespace ECS
{
class ActorComponentVector;

namespace Components
{
class BlackboardComponentInfo;

/**
 * A BlackboardComponent contains a blackboard: a data-driven key/value store.
 */
class BlackboardComponent final : public ActorComponent
{
public:
	using Info = BlackboardComponentInfo;

	static bool TryCreateFromInfo(const BlackboardComponentInfo& componentInfo, const ActorComponentID reservedID,
		ActorComponentVector& destination);

	explicit BlackboardComponent(const ActorComponentID id)
		: ActorComponent(id)
		, m_blackboard()
	{}

	virtual ~BlackboardComponent() {}

	Behave::Blackboard m_blackboard;
};
}
}


