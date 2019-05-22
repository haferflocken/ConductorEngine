#pragma once

#include <chrono>
#include <cstdint>

#define AMP_PROFILING_ENABLED 1

#if AMP_PROFILING_ENABLED == 1
#define AMP_PROFILE_SCOPE() ::Dev::ScopedFrame profilerFrame{ __func__ }
#define AMP_INIT_PROFILER() ::Dev::ScopedProfilerThread scopedProfilerThread
#else
#define AMP_PROFILE_SCOPE()
#define AMP_INIT_PROFILER()
#endif

namespace Dev
{
struct FrameRecord final
{
	uint64_t m_parentFrameID;
	uint64_t m_beginPoint;
	uint64_t m_durationNanoseconds;
	uint64_t m_depth;
	const char* m_frameName;
};

// Push a new frame and return the ID of the previous one.
uint64_t PushFrame(const char* const name, const uint64_t beginNanoseconds);
// Pop the current frame and restore the previous one.
void PopFrame(const uint64_t endNanoseconds, const uint64_t previousFrameID);

/**
 * Pushes a frame on construction; pops a frame on destruction.
 */
class ScopedFrame final
{
	std::chrono::steady_clock::time_point m_beginPoint;
	uint64_t m_previousFrameID;

public:
	ScopedFrame(const char* const name)
		: m_beginPoint(std::chrono::steady_clock::now())
	{
		const std::chrono::nanoseconds time = std::chrono::steady_clock::now().time_since_epoch();
		m_previousFrameID = PushFrame(name, time.count());
	}
	
	~ScopedFrame()
	{
		const std::chrono::nanoseconds time = std::chrono::steady_clock::now().time_since_epoch();
		PopFrame(time.count(), m_previousFrameID);
	}
};

/**
 * Initializes the profiler thread on construction and shuts it down on destruction.
 */
class ScopedProfilerThread final
{
public:
	ScopedProfilerThread();
	~ScopedProfilerThread();
};
}
