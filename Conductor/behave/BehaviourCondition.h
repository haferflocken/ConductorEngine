#pragma once

namespace ECS
{
class Actor;
}

namespace Behave
{
class BehaviourCondition
{
public:
	virtual bool Check(ECS::Actor& actor) const = 0;
};
}
