#pragma once

namespace Behave
{
class Actor;

class BehaviourCondition
{
public:
	virtual bool Check(Actor& actor) const = 0;
};
}
