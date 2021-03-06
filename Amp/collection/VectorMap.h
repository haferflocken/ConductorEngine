#pragma once

#include <collection/Pair.h>
#include <collection/Vector.h>

#include <algorithm>
#include <functional>
#include <utility>

namespace Collection
{
/**
 * A key-value map backed by a sorted vector. Insertions are O(nlogn), finds are O(logn), removes are O(n).
 */
template <typename KeyType, typename ValueType, typename ComparisonType = std::less<KeyType>>
class VectorMap
{
public:
	using value_type = Pair<const KeyType, ValueType>;
	using iterator = value_type*;
	using const_iterator = const value_type*;

	VectorMap() = default;

	VectorMap(const VectorMap& o) = default;
	VectorMap& operator=(const VectorMap& rhs) = default;

	VectorMap(VectorMap&& o) noexcept;
	VectorMap& operator=(VectorMap&& o) noexcept;

	uint32_t Size() const { return m_vector.Size(); }
	uint32_t Capacity() const { return m_vector.Capacity(); }
	bool IsEmpty() const { return m_vector.IsEmpty(); }

	ValueType& operator[](const KeyType& key);
	ValueType& operator[](KeyType&& key);

	iterator Find(const KeyType& key);
	const_iterator Find(const KeyType& key) const;

	bool TryRemove(const KeyType& key);
	bool TryRemove(const KeyType& key, ValueType& outRemovedValue);

	template <typename Predicate>
	void RemoveAllMatching(Predicate&& pred);

	void Clear() { m_vector.Clear(); }

	iterator begin() { return reinterpret_cast<value_type*>(m_vector.begin()); }
	const_iterator begin() const { return reinterpret_cast<const value_type*>(m_vector.begin()); }
	const_iterator cbegin() const { return begin(); }

	iterator end() { return reinterpret_cast<value_type*>(m_vector.end()); }
	const_iterator end() const {  return reinterpret_cast<const value_type*>(m_vector.end()); }
	const_iterator cend() const { return end(); }

private:
	// TODO(refactor) separate key and value types into separate vectors
	Vector<Pair<KeyType, ValueType>> m_vector;
};

template <typename KeyType, typename ValueType, typename ComparisonType>
inline VectorMap<KeyType, ValueType, ComparisonType>::VectorMap(VectorMap&& o) noexcept
	: m_vector(std::move(o.m_vector))
{}

template <typename KeyType, typename ValueType, typename ComparisonType>
inline VectorMap<KeyType, ValueType, ComparisonType>& VectorMap<KeyType, ValueType, ComparisonType>::operator=(
	VectorMap&& o) noexcept
{
	m_vector = std::move(o.m_vector);
	return *this;
}

template <typename KeyType, typename ValueType, typename ComparisonType>
inline ValueType& VectorMap<KeyType, ValueType, ComparisonType>::operator[](const KeyType& key)
{
	iterator i = Find(key);
	if (i == end())
	{
		const auto destinationItr = std::lower_bound(begin(), end(), key, [](const value_type& a, const KeyType& b)
		{
			return (a.first < b);
		});
		const size_t destinationIndex = static_cast<size_t>(destinationItr - begin());

		m_vector.EmplaceAt(destinationIndex, key, ValueType());

		// destinationItr may be invalidated by EmplaceAt.
		i = begin() + destinationIndex;
	}
	return i->second;
}

template <typename KeyType, typename ValueType, typename ComparisonType>
inline ValueType& VectorMap<KeyType, ValueType, ComparisonType>::operator[](KeyType&& key)
{
	iterator i = Find(key);
	if (i == end())
	{
		const auto destinationItr = std::lower_bound(begin(), end(), key, [](const value_type& a, const KeyType& b)
		{
			return (a.first < b);
		});
		const size_t destinationIndex = static_cast<size_t>(destinationItr - begin());

		m_vector.EmplaceAt(destinationIndex, std::move(key), ValueType());

		// destinationItr may be invalidated by EmplaceAt.
		i = begin() + destinationIndex;
	}
	return i->second;
}

template <typename KeyType, typename ValueType, typename ComparisonType>
inline typename VectorMap<KeyType, ValueType, ComparisonType>::iterator
	VectorMap<KeyType, ValueType, ComparisonType>::Find(const KeyType& key)
{
	const auto itr = std::lower_bound(begin(), end(), key, [](const value_type& a, const KeyType& b)
	{
		return (a.first < b);
	});

	return (itr != end() && itr->first == key) ? itr : end();
}

template <typename KeyType, typename ValueType, typename ComparisonType>
inline typename VectorMap<KeyType, ValueType, ComparisonType>::const_iterator
	VectorMap<KeyType, ValueType, ComparisonType>::Find(const KeyType& key) const
{
	const auto itr = std::lower_bound(begin(), end(), key, [](const value_type& a, const KeyType& b)
	{
		return (a.first < b);
	});

	return (itr != end() && itr->first == key) ? itr : end();
}

template <typename KeyType, typename ValueType, typename ComparisonType>
inline bool VectorMap<KeyType, ValueType, ComparisonType>::TryRemove(const KeyType& key)
{
	const iterator itr = Find(key);
	if (itr == end())
	{
		return false;
	}
	const size_t i = std::distance(begin(), itr);
	m_vector.Remove(i, i + 1);
	return true;
}

template <typename KeyType, typename ValueType, typename ComparisonType>
inline bool VectorMap<KeyType, ValueType, ComparisonType>::TryRemove(const KeyType& key, ValueType& outRemovedValue)
{
	const iterator itr = Find(key);
	if (itr == end())
	{
		return false;
	}

	outRemovedValue = std::move(itr->second);

	const size_t i = std::distance(begin(), itr);
	m_vector.Remove(i, i + 1);
	return true;
}

template <typename KeyType, typename ValueType, typename ComparisonType>
template <typename Predicate>
inline void VectorMap<KeyType, ValueType, ComparisonType>::RemoveAllMatching(Predicate&& pred)
{
	const auto iter = std::remove_if(m_vector.begin(), m_vector.end(), std::forward<Predicate>(pred));
	const size_t i = std::distance(m_vector.begin(), iter);
	m_vector.Remove(i, m_vector.Size());
}
}
