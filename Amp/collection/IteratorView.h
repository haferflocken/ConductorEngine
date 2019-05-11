#pragma once

namespace Collection
{
/**
 * Wraps a pair of iterators as if they together were an iterable container.
 */
template <typename IteratorType>
class IteratorView
{
public:
	using iterator = IteratorType;
	using const_iterator = IteratorType;

	IteratorView(IteratorType begin, IteratorType end)
		: m_begin(begin)
		, m_end(end)
	{}

	IteratorType begin() const { return m_begin; }
	IteratorType cbegin() const { return m_begin; }

	IteratorType end() const { return m_end; }
	IteratorType cend() const { return m_end; }

private:
	IteratorType m_begin;
	IteratorType m_end;
};
}
