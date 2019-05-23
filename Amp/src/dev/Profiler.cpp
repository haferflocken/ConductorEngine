#include <dev/Profiler.h>

#include <collection/LinearBlockHashMap.h>
#include <collection/LocklessQueue.h>
#include <mem/UniquePtr.h>

#include <array>

namespace Internal_Profiler
{
/**
 * ProfilerEvent stores a push or pop event for a profiling frame.
 */
class ProfilerEvent final
{
	// The high bit is set if this is a pop event.
	uint64_t m_frameID;
	// The time at which this event occurred.
	uint64_t m_timePoint;
	// If this is a push event, this is a pointer to the frame name.
	uint64_t m_payload;

	ProfilerEvent(const uint64_t frameID, const uint64_t timePoint, const uint64_t payload)
		: m_frameID(frameID)
		, m_timePoint(timePoint)
		, m_payload(payload)
	{}

public:
	static ProfilerEvent MakeNullEvent()
	{
		return ProfilerEvent(UINT64_MAX, UINT64_MAX, UINT64_MAX);
	}

	static ProfilerEvent MakePushEvent(const uint64_t frameID, const uint64_t timePoint, const char* const frameName)
	{
		return ProfilerEvent{ frameID, timePoint, reinterpret_cast<uint64_t>(frameName) };
	}

	static ProfilerEvent MakePopEvent(const uint64_t frameID, const uint64_t timePoint)
	{
		return ProfilerEvent{ frameID | (1ui64 << 63), timePoint, 0 };
	}

	bool IsPushEvent() const { return (m_frameID & (1ui64 << 63)) == 0; }
	uint64_t GetFrameID() const { return m_frameID & ~(1ui64 << 63); }
	uint64_t GetTimePoint() const { return m_timePoint; }
	const char* GetFrameName() const { return reinterpret_cast<const char*>(m_payload); }
};

/**
 * ThreadLocalProfiler stores the thread-local data for a push/pop event queue.
 */
struct ThreadLocalProfiler final
{
	// The ID that will be given to the next PushFrame() call.
	uint64_t m_nextFrameID{ 1 };
	// The ID of the currently active frame.
	uint64_t m_currentFrameID{ 0 };
	// A fixed size buffer of push & pop events.
	Collection::LocklessQueue<ProfilerEvent> m_eventQueue{ 0xFFFF };
};

/**
 * FrameRecordMap is used to process the profiler event queues for a thread and stores the result.
 */
class FrameRecordMap final
{
public:
	FrameRecordMap()
		: m_currentFrameID(0)
		, m_currentDepth(1)
		, m_numRecordsInLastBlock(1)
		, m_frameRecordBlocks(32)
	{
		m_frameRecordBlocks.Add(Mem::MakeUnique<FrameRecordBlock>());
		FrameRecordBlock& block = *m_frameRecordBlocks.Back();
		Dev::FrameRecord& rootRecord = block.front();
		rootRecord.m_parentFrameID = 0;
		rootRecord.m_beginPoint = 0;
		rootRecord.m_durationNanoseconds = UINT64_MAX;
		rootRecord.m_depth = 0;
		rootRecord.m_frameName = "FrameRecordMap Root Frame";
	}

	void PushFrame(uint64_t frameID, uint64_t beginPoint, const char* name)
	{
		AMP_ASSERT(frameID == GetNumFrameRecords(), "");

		const uint64_t parentFrameID = m_currentFrameID;
		m_currentFrameID = frameID;

		Dev::FrameRecord& frameRecord = CreateFrameRecord();
		frameRecord.m_parentFrameID = parentFrameID;
		frameRecord.m_beginPoint = beginPoint;
		frameRecord.m_durationNanoseconds = UINT64_MAX;
		frameRecord.m_depth = m_currentDepth;
		frameRecord.m_frameName = name;

		++m_currentDepth;
	}

	void PopFrame(uint64_t frameID, uint64_t endPoint)
	{
		AMP_ASSERT(frameID == m_currentFrameID, "");
		Dev::FrameRecord& frameRecord = GetFrameRecord(frameID);
		frameRecord.m_durationNanoseconds = endPoint - frameRecord.m_beginPoint;

		m_currentFrameID = frameRecord.m_parentFrameID;
		--m_currentDepth;
	}

	Dev::FrameRecord& GetFrameRecord(uint64_t frameID)
	{
		const size_t blockIndex = frameID / k_frameRecordBlockSize;
		const size_t indexInBlock = frameID % k_frameRecordBlockSize;
		FrameRecordBlock& block = *m_frameRecordBlocks[blockIndex];
		return block[indexInBlock];
	}

	uint32_t GetNumFrameRecords() const
	{
		return (m_frameRecordBlocks.Size() - 1) * k_frameRecordBlockSize + m_numRecordsInLastBlock;
	}

private:
	static constexpr size_t k_frameRecordBlockSize = 4096;
	using FrameRecordBlock = std::array<Dev::FrameRecord, k_frameRecordBlockSize>;

	Dev::FrameRecord& CreateFrameRecord()
	{
		if (m_numRecordsInLastBlock < k_frameRecordBlockSize)
		{
			const uint32_t indexInBlock = m_numRecordsInLastBlock;
			++m_numRecordsInLastBlock;
			FrameRecordBlock& block = *m_frameRecordBlocks.Back();
			return block[indexInBlock];
		}

		m_numRecordsInLastBlock = 1;
		m_frameRecordBlocks.Add(Mem::MakeUnique<FrameRecordBlock>());
		FrameRecordBlock& block = *m_frameRecordBlocks.Back();
		return block.front();
	}

private:
	uint64_t m_currentFrameID;
	uint64_t m_currentDepth;
	// Because frame IDs increment upwards from 0, we can use them as indices.
	// We allocate frame records in blocks of 4096 that don't move around to avoid copying them.
	uint32_t m_numRecordsInLastBlock;
	Collection::Vector<Mem::UniquePtr<FrameRecordBlock>> m_frameRecordBlocks; 
};


thread_local ThreadLocalProfiler* tls_profiler = nullptr;
Collection::LinearBlockHashMap<uint64_t, ThreadLocalProfiler, Collection::I64HashFunctor> g_profilerRegistry{ Collection::I64HashFunctor(), 4 };
Collection::LinearBlockHashMap<uint64_t, FrameRecordMap, Collection::I64HashFunctor> g_frameRecordsByRegistryID{ Collection::I64HashFunctor(), 4 };
uint64_t g_nextProfilerRegistryID = 0;
std::mutex g_profilerRegistryMutex;

std::thread g_profilerThread;
bool g_shouldRunProfilerThread = false;

// Transform the thread-local event queues into frame records.
void ProcessProfilerEventQueues()
{
	std::unique_lock<std::mutex> lock{ g_profilerRegistryMutex };

	for (auto&& entry : g_profilerRegistry.GetKeyValueView())
	{
		const uint64_t profiledThreadID = entry.key;
		ThreadLocalProfiler& profiler = *entry.value;
		FrameRecordMap& frameRecordMap = *g_frameRecordsByRegistryID.Find(profiledThreadID);

		ProfilerEvent event = ProfilerEvent::MakeNullEvent();
		while (profiler.m_eventQueue.TryPop(event))
		{
			const uint64_t frameID = event.GetFrameID();
			const uint64_t timePoint = event.GetTimePoint();

			if (event.IsPushEvent())
			{
				const char* const frameName = event.GetFrameName();
				frameRecordMap.PushFrame(frameID, timePoint, frameName);
			}
			else
			{
				frameRecordMap.PopFrame(frameID, timePoint);
			}
		}
	}
}

void ProfilerThreadFunction()
{
	while (g_shouldRunProfilerThread)
	{
		ProcessProfilerEventQueues();
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}
}
}

uint64_t Dev::PushFrame(const char* const name, const uint64_t beginNanoseconds)
{
	using namespace Internal_Profiler;

	// Ensure the thread is registered with the global list of profiled threads.
	if (tls_profiler == nullptr)
	{
		std::unique_lock<std::mutex> lock{ g_profilerRegistryMutex };
		tls_profiler = &g_profilerRegistry.Emplace(g_nextProfilerRegistryID);
		g_frameRecordsByRegistryID.Emplace(g_nextProfilerRegistryID);
		++g_nextProfilerRegistryID;
	}

	// Reserve a frame ID, push it, and return the ID of the previous one.
	ThreadLocalProfiler& profiler = *tls_profiler;
	const uint64_t frameID = profiler.m_nextFrameID;
	++profiler.m_nextFrameID;

	profiler.m_eventQueue.TryPush(ProfilerEvent::MakePushEvent(frameID, beginNanoseconds, name));

	const uint64_t previousFrameID = profiler.m_currentFrameID;
	profiler.m_currentFrameID = frameID;

	return previousFrameID;
}

void Dev::PopFrame(const uint64_t endNanoseconds, const uint64_t previousFrameID)
{
	using namespace Internal_Profiler;

	// Store a pop event and restore the ID of the previous frame.
	ThreadLocalProfiler& profiler = *tls_profiler;
	profiler.m_eventQueue.TryPush(ProfilerEvent::MakePopEvent(profiler.m_currentFrameID, endNanoseconds));
	profiler.m_currentFrameID = previousFrameID;
}

Dev::ScopedProfilerThread::ScopedProfilerThread()
{
	using namespace Internal_Profiler;
	g_shouldRunProfilerThread = true;
	g_profilerThread = std::thread(&ProfilerThreadFunction);
}

Dev::ScopedProfilerThread::~ScopedProfilerThread()
{
	using namespace Internal_Profiler;
	g_shouldRunProfilerThread = false;
	g_profilerThread.join();
}
