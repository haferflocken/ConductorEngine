#pragma once

#include <traits/IsMemCopyAFullCopy.h>

#include <iterator>

namespace Collection
{
template <typename IntegralType>
class IntegralRangeIterator
{
	IntegralType m_value;

public:
	using value_type = IntegralType;
	using reference = const IntegralType&;
	using iterator_category = std::input_iterator_tag;

	explicit IntegralRangeIterator(IntegralType val)
		: m_value(val)
	{}

	bool operator==(const IntegralRangeIterator& rhs) const { return m_value == rhs.m_value; }
	bool operator!=(const IntegralRangeIterator& rhs) const { return m_value != rhs.m_value; }

	reference operator*() const { return m_value; }
	reference operator->() const { return m_value; }

	IntegralRangeIterator& operator++()
	{
		++m_value;
		return *this;
	}
};

template <typename IntegralType>
class IntegralRange
{
	IntegralType m_first;
	IntegralType m_last;

public:
	using const_iterator = IntegralRangeIterator<IntegralType>;

	IntegralRange(IntegralType first, IntegralType last)
		: m_first(first)
		, m_last(last)
	{}

	const IntegralType& GetFirst() const { return m_first; }
	const IntegralType& GetLast() const { return m_last; }

	const_iterator begin() const { return IntegralRangeIterator<IntegralType>(m_first); }
	const_iterator cbegin() const { return begin(); }

	const_iterator end() const { return IntegralRangeIterator<IntegralType>(m_last + 1); }
	const_iterator cend() const { return end(); }
};
}

namespace Traits
{
	/**
	* IsMemCopyAFullCopy specialization to indicate that an IntegralRange can be memcpy'd if its backing type can.
	*/
	template<typename IntegralType>
	struct IsMemCopyAFullCopy<Collection::IntegralRange<IntegralType>> : IsMemCopyAFullCopy<IntegralType> {};
}
