#pragma once

namespace ECS { class Entity; }

namespace Behave
{
class BehaviourCondition
{
public:
	virtual bool Check(ECS::Entity& entity) const = 0;
};
}
