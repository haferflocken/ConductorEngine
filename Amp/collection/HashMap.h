#pragma once

#include <collection/ArrayView.h>
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
	struct BucketView
	{
		Collection::ArrayView<const KeyType> m_keys;
		Collection::ArrayView<ValueType> m_values;
	};
	struct ConstBucketView
	{
		Collection::ArrayView<const KeyType> m_keys;
		Collection::ArrayView<const ValueType> m_values;
	};

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

	BucketView GetBucketView(const KeyType& key);
	ConstBucketView GetBucketView(const KeyType& key) const;

	BucketView GetBucketViewAt(size_t index);
	ConstBucketView GetBucketViewAt(size_t index) const;

	size_t GetNumBuckets() const;

	void Clear();
	
	bool TryRemove(const KeyType& key, ValueType* outRemovedValue = nullptr);

private:
	HashFn m_hashFunction;
	uint64_t m_shift;

	struct Bucket
	{
		Collection::Vector<KeyType> m_keys;
		Collection::Vector<ValueType> m_values;
	};
	Collection::Vector<Bucket> m_buckets;
};

/**
 * A hash functor implementation for 64 bit integers.
 */
class I64HashFunctor
{
	int64_t m_a;
	int64_t m_b;

public:
	I64HashFunctor();

	uint64_t Hash(uint64_t key) const { return m_a * key + m_b; }
	uint64_t Hash(int64_t key) const { return m_a * key + m_b; }
	void Rehash();
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
	for (size_t i = 0, iEnd = bucket.m_keys.Size(); i < iEnd; ++i)
	{
		if (bucket.m_keys[i] == key)
		{
			return bucket.m_values[i];
		}
	}

	bucket.m_keys.Add(key);
	return bucket.m_values.Emplace();
}

template <typename KeyType, typename ValueType, typename HashFn>
inline ValueType& HashMap<KeyType, ValueType, HashFn>::operator[](KeyType&& key)
{
	const uint64_t hash = m_hashFunction.Hash(key);
	const uint64_t bucketIndex = hash >> m_shift;
	Bucket& bucket = m_buckets[bucketIndex];
	for (size_t i = 0, iEnd = bucket.m_keys.Size(); i < iEnd; ++i)
	{
		if (bucket.m_keys[i] == key)
		{
			return bucket.m_values[i];
		}
	}

	bucket.m_keys.Add(std::move(key));
	return bucket.m_values.Emplace();
}

template <typename KeyType, typename ValueType, typename HashFn>
inline ValueType* HashMap<KeyType, ValueType, HashFn>::Find(const KeyType& key)
{
	return const_cast<ValueType*>(static_cast<const HashMap<KeyType, ValueType, HashFn>*>(this)->Find(key));
}

template <typename KeyType, typename ValueType, typename HashFn>
inline const ValueType* HashMap<KeyType, ValueType, HashFn>::Find(const KeyType& key) const
{
	const uint64_t hash = m_hashFunction.Hash(key);
	const uint64_t bucketIndex = hash >> m_shift;
	const Bucket& bucket = m_buckets[bucketIndex];
	for (size_t i = 0, iEnd = bucket.m_keys.Size(); i < iEnd; ++i)
	{
		if (bucket.m_keys[i] == key)
		{
			return &bucket.m_values[i];
		}
	}
	return nullptr;
}

template <typename KeyType, typename ValueType, typename HashFn>
inline typename HashMap<KeyType, ValueType, HashFn>::BucketView HashMap<KeyType, ValueType, HashFn>::GetBucketView(
	const KeyType& key)
{
	const uint64_t hash = m_hashFunction.Hash(key);
	const uint64_t bucketIndex = hash >> m_shift;
	Bucket& bucket = m_buckets[bucketIndex];
	return BucketView{ bucket.m_keys.GetConstView(), bucket.m_values.GetView() };
}

template <typename KeyType, typename ValueType, typename HashFn>
inline typename HashMap<KeyType, ValueType, HashFn>::ConstBucketView HashMap<KeyType, ValueType, HashFn>::GetBucketView(
	const KeyType& key) const
{
	const uint64_t hash = m_hashFunction.Hash(key);
	const uint64_t bucketIndex = hash >> m_shift;
	const Bucket& bucket = m_buckets[bucketIndex];
	return ConstBucketView{ bucket.m_keys.GetConstView(), bucket.m_values.GetView() };
}

template <typename KeyType, typename ValueType, typename HashFn>
inline typename HashMap<KeyType, ValueType, HashFn>::BucketView HashMap<KeyType, ValueType, HashFn>::GetBucketViewAt(
	size_t index)
{
	Bucket& bucket = m_buckets[index];
	return BucketView{ bucket.m_keys.GetConstView(), bucket.m_values.GetView() };
}

template <typename KeyType, typename ValueType, typename HashFn>
inline typename HashMap<KeyType, ValueType, HashFn>::ConstBucketView HashMap<KeyType, ValueType, HashFn>::GetBucketViewAt(
	size_t index) const
{
	const Bucket& bucket = m_buckets[index];
	return ConstBucketView{ bucket.m_keys.GetConstView(), bucket.m_values.GetView() };
}

template <typename KeyType, typename ValueType, typename HashFn>
inline size_t HashMap<KeyType, ValueType, HashFn>::GetNumBuckets() const
{
	return m_buckets.Size();
}

template <typename KeyType, typename ValueType, typename HashFn>
inline void HashMap<KeyType, ValueType, HashFn>::Clear()
{
	for (auto& bucket : m_buckets)
	{
		bucket.m_keys.Clear();
		bucket.m_values.Clear();
	}
}

template <typename KeyType, typename ValueType, typename HashFn>
inline bool HashMap<KeyType, ValueType, HashFn>::TryRemove(const KeyType& key, ValueType* outRemovedValue)
{
	const uint64_t hash = m_hashFunction.Hash(key);
	const uint64_t bucketIndex = hash >> m_shift;
	Bucket& bucket = m_buckets[bucketIndex];
	for (size_t i = 0, iEnd = bucket.m_keys.Size(); i < iEnd; ++i)
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
