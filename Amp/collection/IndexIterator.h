#pragma once

#include <dev/Dev.h>

#include <cstdint>
#include <iterator>
#include <type_traits>

namespace Collection
{
/**
 * A helper class for defining iterators that can iterate by index.
 */
template <typename ContainerType, typename QualifiedValueType>
class IndexIterator
{
public:
	using difference_type = int64_t;
	using value_type = std::remove_cv_t<QualifiedValueType>;
	using pointer = QualifiedValueType*;
	using reference = QualifiedValueType&;
	using iterator_category = std::random_access_iterator_tag;

	IndexIterator() = default;

	IndexIterator(ContainerType& container, int64_t index)
		: m_container(&container)
		, m_index(index)
	{}

	int64_t GetIndex() const { return m_index; }

	IndexIterator& operator++() { ++m_index; return *this; }
	IndexIterator operator++(int) { IndexIterator temp = *this; ++m_index; return temp; }

	IndexIterator& operator--() { --m_index; return *this; }
	IndexIterator operator--(int) { IndexIterator temp = *this; --m_index; return temp; }

	IndexIterator& operator+=(difference_type n) { m_index += n; return *this; }
	IndexIterator& operator-=(difference_type n) { m_index -= n; return *this; }

	IndexIterator operator+(difference_type rhs) const { IndexIterator temp = *this; return temp += rhs; }
	IndexIterator operator-(difference_type rhs) const { IndexIterator temp = *this; return temp -= rhs; }

	difference_type operator-(const IndexIterator& rhs) const { return m_index - rhs.m_index; }

	bool operator==(const IndexIterator& rhs) const { return m_container == rhs.m_container && m_index == rhs.m_index; }
	bool operator!=(const IndexIterator& rhs) const { return m_container != rhs.m_container ||  m_index != rhs.m_index; }
	bool operator<(const IndexIterator& rhs) const { return m_container == rhs.m_container && m_index < rhs.m_index; }

	bool operator>(const IndexIterator& rhs) const { return rhs < *this; }
	bool operator>=(const IndexIterator& rhs) const { return !(*this < rhs); }
	bool operator<=(const IndexIterator& rhs) const { return !(*this > rhs); }

	reference operator[](difference_type n) { return (*m_container)[m_index + n]; }

	reference operator*() { return (*m_container)[m_index]; }
	const reference operator*() const { return (*m_container)[m_index]; }

	pointer operator->() { return &((*m_container)[m_index]); }
	const pointer operator->() const { return &((*m_container)[m_index]); }

private:
	ContainerType* m_container{ nullptr };
	int64_t m_index{ 0 };
};
}

template <typename ContainerType, typename QualifiedValueType>
Collection::IndexIterator<ContainerType, QualifiedValueType> operator+(
	typename Collection::IndexIterator<ContainerType, QualifiedValueType>::difference_type lhs,
	const Collection::IndexIterator<ContainerType, QualifiedValueType>& rhs)
{
	return rhs + lhs;
}
