#pragma once

#include <collection/Vector.h>

namespace Collection
{
/**
 * A key-value map with constant-time (amortized) access.
 * The provided HashFn must be:
 * - Copyable or moveable
 * - Implement a hash function: uint64_t Hash(const KeyType& key) const;
 * - Implement a reset function: void Rehash(); This should change the hash function so that
 *   future calls to Hash produce new values for old keys.
 */
template <typename KeyType, typename ValueType, typename HashFn>
class HashMap
{
public:
	class iterator;
	class const_iterator;

	HashMap(HashFn&& hashFn, uint32_t numBucketsShift)
		: m_hashFunction(std::move(hashFn))
		, m_shift(64 - numBucketsShift)
		, m_buckets(1u << numBucketsShift)
	{
		m_buckets.Resize(1u << numBucketsShift);
	}

	ValueType& operator[](const KeyType& key);
	ValueType& operator[](KeyType&& key);
	
	ValueType* Find(const KeyType& key);
	const ValueType* Find(const KeyType& key) const;
	
	bool TryRemove(const KeyType& key, ValueType* outRemovedValue = nullptr);

	/*iterator begin();
	const_iterator begin() const;
	const_iterator cbegin() const;

	iterator end();
	const_iterator end() const;
	const_iterator cend() const;*/

private:
	HashFn m_hashFunction;
	uint64_t m_shift;

	struct Bucket
	{
		Collection::Vector<KeyType> m_keys{};
		Collection::Vector<ValueType> m_values{};
	};
	Collection::Vector<Bucket> m_buckets;
};
}

// Inline implementation.
namespace Collection
{
template <typename KeyType, typename ValueType, typename HashFn>
inline ValueType& HashMap<KeyType, ValueType, HashFn>::operator[](const KeyType& key)
{
	const uint64_t hash = m_hashFunction.Hash(key);
	const uint64_t bucketIndex = hash >> m_shift;
	Bucket& bucket = m_buckets[bucketIndex];
	bucket.m_keys.Add(key);
	bucket.m_values.Add(value);
}

template <typename KeyType, typename ValueType, typename HashFn>
inline ValueType& HashMap<KeyType, ValueType, HashFn>::operator[](KeyType&& key)
{
	const uint64_t hash = m_hashFunction.Hash(key);
	const uint64_t bucketIndex = hash >> m_shift;
	Bucket& bucket = m_buckets[bucketIndex];
	bucket.m_keys.Add(std::move(key));
	bucket.m_values.Add(value);
}

template <typename KeyType, typename ValueType, typename HashFn>
inline ValueType* HashMap<KeyType, ValueType, HashFn>::Find(const KeyType& key)
{
	return const_cast<ValueType*>(static_cast<const HashMap<KeyType, ValueType>*>(this)->Find(key));
}

template <typename KeyType, typename ValueType, typename HashFn>
inline const ValueType* HashMap<KeyType, ValueType, HashFn>::Find(const KeyType& key) const
{
	const uint64_t hash = m_hashFunction.Hash(key);
	const uint64_t bucketIndex = hash >> m_shift;
	const Bucket& bucket = m_buckets[bucketIndex];
	for (size_t i = 0, iEnd = bucket.m_keys.size(); i < iEnd; ++i)
	{
		if (bucket.m_keys[i] == key)
		{
			return &bucket.m_values[i];
		}
	}
	return nullptr;
}

template <typename KeyType, typename ValueType, typename HashFn>
inline bool HashMap<KeyType, ValueType, HashFn>::TryRemove(const KeyType& key, ValueType* outRemovedValue)
{
	const uint64_t hash = m_hashFunction.Hash(key);
	const uint64_t bucketIndex = hash >> m_shift;
	Bucket& bucket = m_buckets[bucketIndex];
	for (size_t i = 0, iEnd = bucket.m_keys.size(); i < iEnd; ++i)
	{
		if (bucket.m_keys[i] == key)
		{
			bucket.m_keys.Remove(i, i + 1);
			if (outRemovedValue != nullptr)
			{
				*outRemovedValue = bucket.m_values[i];
			}
			bucket.m_values.Remove(i, i + 1);
			return true;
		}
	}
	return false;
}
}
