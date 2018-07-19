#pragma once

#include <ecs/ActorComponentID.h>

namespace ECS
{
/**
 * An ActorComponent holds data for an actor. A component is always instantiated using an ActorComponentInfo.
 * ActorComponents must define an Info type which they are instantiated from, and must define a TryCreateFromInfo
 * static function which creates them from their info.
 */
class ActorComponent
{
public:
	explicit ActorComponent(const ActorComponentID id)
		: m_id(id)
	{}

	virtual ~ActorComponent() {}

	ActorComponentID m_id;
};
}
