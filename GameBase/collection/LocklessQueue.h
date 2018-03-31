#pragma once

#include <collection/Vector.h>

#include <atomic>

namespace Collection
{
/**
 * A lockless, thread safe, single producer/single consumer queue with a fixed capacity.
 */
template <typename T>
class LocklessQueue
{
public:

	LocklessQueue(size_t capacity)
		: m_queue(static_cast<T*>(malloc(sizeof(T) * capacity)))
		, m_inputIndex(0)
		, m_outputIndex(0)
		, m_capacity(capacity)
	{}

	~LocklessQueue()
	{
		free(m_queue);
		m_queue = nullptr;
	}

	bool TryPush(T&& item)
	{
		const size_t nextInputIndex = NextIndex(m_inputIndex);
		if (nextInputIndex == m_outputIndex)
		{
			return false;
		}

		new (&m_queue[m_inputIndex]) T(std::move(item));
		m_inputIndex = nextInputIndex;
		return true;
	}

	bool TryPop(T& outItem)
	{
		if (m_outputIndex == m_inputIndex)
		{
			return false;
		}

		outItem = std::move(m_queue[m_outputIndex]);
		m_queue[m_outputIndex].~T();
		m_outputIndex = NextIndex(m_outputIndex);
		return true;
	}

private:
	size_t NextIndex(size_t index)
	{
		return (index + 1) % m_capacity;
	}

	T* m_queue;
	size_t m_inputIndex;
	size_t m_outputIndex;
	size_t m_capacity; // Not atomic because its value does not change.
};
}
