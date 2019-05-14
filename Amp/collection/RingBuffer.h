#pragma once

#include <dev/Dev.h>

#include <array>

namespace Collection
{
/**
 * A fixed-capacity ring buffer.
 */
template <typename T, size_t S>
class RingBuffer
{
public:
	using value_type = T;
	static constexpr size_t k_capacity = S;

	RingBuffer() = default;

	RingBuffer(RingBuffer&&);
	RingBuffer& operator=(RingBuffer&&);

	~RingBuffer() = default;

	size_t Size() const;

	// Advances the buffer's indices as if an element was added to it. Returns the oldest element if the buffer is at
	// capacity. Otherwise, returns an unused element.
	T& AddRecycle();

	// Add an element to the buffer, overwriting the oldest element if the buffer is at capacity.
	void Add(const T& element);
	void Add(T&& element);

	// Access the buffer by index. 0 is the oldest element.
	T& operator[](size_t i);
	const T& operator[](size_t i) const;

	// Access the newest element of the buffer.
	T& Newest();
	const T& Newest() const;

private:
	size_t m_beginIndex{ SIZE_MAX }; // The index of the oldest element.
	size_t m_endIndex{ 0 }; // The index of the next insertion.
	std::array<T, k_capacity> m_buffer;
};
}

// Inline implementations.
namespace Collection
{
template <typename T, size_t S>
inline RingBuffer<T, S>::RingBuffer(RingBuffer&& other)
	: m_beginIndex(other.m_beginIndex)
	, m_endIndex(other.m_endIndex)
	, m_buffer(std::move(other.m_buffer))
{}

template <typename T, size_t S>
inline RingBuffer<T, S>& RingBuffer<T, S>::operator=(RingBuffer&& rhs)
{
	m_beginIndex = rhs.m_beginIndex;
	m_endIndex = rhs.m_endIndex;
	m_buffer = std::move(rhs.m_buffer);
	return *this;
}

template <typename T, size_t S>
inline size_t RingBuffer<T, S>::Size() const
{
	if (m_beginIndex == SIZE_MAX)
	{
		return 0;
	}
	const size_t e = (m_endIndex >= m_beginIndex) ? m_endIndex : (m_endIndex + k_capacity);
	const size_t size = e - m_beginIndex;
	return size;
}

template <typename T, size_t S>
inline T& RingBuffer<T, S>::AddRecycle()
{
	if (m_beginIndex == SIZE_MAX)
	{
		m_beginIndex = 0;
		m_endIndex = 1;
		return m_buffer[0];
	}
	else
	{
		T& result = m_buffer[m_endIndex];

		const size_t size = Size();
		if (size >= k_capacity)
		{
			++m_beginIndex;
			m_beginIndex %= k_capacity;
		}

		++m_endIndex;
		m_endIndex %= k_capacity;

		return result;
	}
}

template <typename T, size_t S>
inline void RingBuffer<T, S>::Add(const T& element)
{
	T& destination = AddRecycle();
	destination = element;
}

template <typename T, size_t S>
inline void RingBuffer<T, S>::Add(T&& element)
{
	T& destination = AddRecycle();
	destination = std::move(element);
}

template <typename T, size_t S>
inline T& RingBuffer<T, S>::operator[](size_t i)
{
	return const_cast<T&>((*static_cast<const RingBuffer*>(this))[i]);
}

template <typename T, size_t S>
inline const T& RingBuffer<T, S>::operator[](size_t i) const
{
	AMP_FATAL_ASSERT(i < Size(), "Index [%zu] is out of bounds!", i);
	const size_t adjustedIndex = (m_beginIndex + i) % k_capacity;
	return m_buffer[adjustedIndex];
}


template <typename T, size_t S>
inline T& RingBuffer<T, S>::Newest()
{
	return const_cast<T&>(static_cast<const RingBuffer*>(this)->Newest());
}

template <typename T, size_t S>
const T& RingBuffer<T, S>::Newest() const
{
	AMP_FATAL_ASSERT(m_beginIndex != SIZE_MAX, "There is no newest element of an empty ring buffer!");
	const size_t i = (m_endIndex + k_capacity - 1) % k_capacity;
	return m_buffer[i];
}
}
