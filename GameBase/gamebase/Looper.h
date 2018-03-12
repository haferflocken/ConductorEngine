#pragma once

#include <unit/Time.h>

#include <ctime>
#include <functional>

namespace GameBase
{
enum class LoopStatus : size_t
{
	Continue,
	Stop,
	Error,
};

enum class LoopTerminationType : size_t
{
	Normal,
	Error,
};

inline LoopTerminationType RunLoop(const std::function<LoopStatus(Unit::Time::Millisecond)>& fn)
{
	// Loop until the loop stops.
	Unit::Time::Millisecond endTime(clock());
	LoopStatus loopStatus = LoopStatus::Continue;
	while (loopStatus == LoopStatus::Continue)
	{
		const Unit::Time::Millisecond startTime(clock());
		loopStatus = fn(startTime - endTime);
		endTime = startTime;
	}
	
	// Turn the final loop status into a LoopTerminationType.
	switch (loopStatus)
	{
	case LoopStatus::Stop:
		return LoopTerminationType::Normal;
	default:
		return LoopTerminationType::Error;
	}
}
}
