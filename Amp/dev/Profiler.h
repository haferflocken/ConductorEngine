#pragma once

#include <collection/Vector.h>
#include <mem/UniquePtr.h>

#include <array>
#include <chrono>
#include <cstdint>
#include <functional>

#define AMP_PROFILING_ENABLED 1

#if AMP_PROFILING_ENABLED == 1
#define AMP_PROFILE_SCOPE() ::Dev::Profiler::ScopedFrame profilerFrame{ __func__ }
#define AMP_INIT_PROFILER() ::Dev::Profiler::ScopedProfilerThread scopedProfilerThread
#else
#define AMP_PROFILE_SCOPE()
#define AMP_INIT_PROFILER()
#endif

namespace Dev::Profiler
{
class FrameRecordMap;

// Push a new frame and return the ID of the previous one.
uint64_t PushFrame(const char* const name, const uint64_t beginNanoseconds);
// Pop the current frame and restore the previous one.
void PopFrame(const uint64_t endNanoseconds, const uint64_t previousFrameID);

// Iterate over the frame record maps. fn is called with (profiler thread ID, frame record map).
// This is the only mechanism to access profiling data.
void ForEachFrameRecordMap(const std::function<void(uint64_t, const FrameRecordMap&)>& fn);

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
 * FrameRecordMap is used to process the profiler event queues for a thread and stores the result.
 */
class FrameRecordMap final
{
public:
	struct FrameRecord final
	{
		uint64_t m_parentFrameID;
		uint64_t m_beginPoint;
		uint64_t m_durationNanoseconds;
		uint64_t m_depth;
		const char* m_frameName;
	};

public:
	FrameRecordMap();

	FrameRecord& GetFrameRecord(uint64_t frameID);
	const FrameRecord& GetFrameRecord(uint64_t frameID) const;

	uint32_t GetNumFrameRecords() const;

	void PushFrame(uint64_t frameID, uint64_t beginPoint, const char* name);
	void PopFrame(uint64_t frameID, uint64_t endPoint);

private:
	static constexpr size_t k_frameRecordBlockSize = 4096;
	using FrameRecordBlock = std::array<FrameRecord, k_frameRecordBlockSize>;

	FrameRecord& CreateFrameRecord();

private:
	uint64_t m_currentFrameID;
	uint64_t m_currentDepth;
	// Because frame IDs increment upwards from 0, we can use them as indices.
	// We allocate frame records in blocks of 4096 that don't move around to avoid copying them.
	uint32_t m_numRecordsInLastBlock;
	Collection::Vector<Mem::UniquePtr<FrameRecordBlock>> m_frameRecordBlocks;
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
