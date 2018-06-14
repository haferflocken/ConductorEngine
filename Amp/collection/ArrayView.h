#pragma once

#include <dev/Dev.h>

namespace Collection
{
/**
 * Provides a view wrapper into collections of contiguous memory.
 */
template <typename T>
class ArrayView
{
public:
	using value_type = T;
	using iterator = T*;
	using const_iterator = const T*;

	ArrayView(T* begin, size_t count)
		: m_data(begin)
		, m_count(count)
	{}

	size_t Size() const { return m_count; }

	iterator begin() { return m_data; }
	const_iterator begin() const { return m_data; }
	const_iterator cbegin() const { return begin(); }

	iterator end() { return m_data + m_count; }
	const_iterator end() const { return m_data + m_count; }
	const_iterator cend() const { return end(); }

	T& operator[](size_t i)
	{
		Dev::FatalAssert(i < m_count, "ArrayView index %zu is out of range: [0, %zu)", i, m_count);
		return m_data[i];
	}

	const T& operator[](size_t i) const
	{
		Dev::FatalAssert(i < m_count, "ArrayView index %zu is out of range: [0, %zu)", i, m_count);
		return m_data[i];
	}

private:
	T* m_data;
	size_t m_count;
};
}
