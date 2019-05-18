#pragma once

#include <collection/ArrayView.h>
#include <collection/IteratorView.h>
#include <collection/Pair.h>
#include <collection/Vector.h>

namespace Collection
{
template <typename Bucket, typename KeyValuePair> class HashMapKeyValueIterator;
template <typename Bucket, typename Key> class HashMapKeyIterator;
template <typename Bucket, typename Value> class HashMapValueIterator;

template <typename KeyType, typename ValueType>
struct HashMapBucket
{
	Collection::Vector<KeyType> m_keys;
	Collection::Vector<ValueType> m_values;
};

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
	using Bucket = HashMapBucket<KeyType, ValueType>;
	using KeyValueIterator = HashMapKeyValueIterator<Bucket, Collection::Pair<const KeyType&, ValueType&>>;
	using ConstKeyValueIterator = HashMapKeyValueIterator<Bucket, Collection::Pair<const KeyType&, const ValueType&>>;
	using KeyIterator = HashMapKeyIterator<Bucket, const KeyType&>;
	using ValueIterator = HashMapValueIterator<Bucket, ValueType>;
	using ConstValueIterator = HashMapValueIterator<Bucket, const ValueType>;

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

public:
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

	IteratorView<KeyValueIterator> GetKeyValueView() { return { KeyValueIterator(m_buckets, 0), KeyValueIterator(m_buckets, m_buckets.Size()) }; }
	IteratorView<ConstKeyValueIterator> GetKeyValueView() const { return { ConstKeyValueIterator(m_buckets, 0), ConstKeyValueIterator(m_buckets, m_buckets.Size()) }; }

	IteratorView<KeyIterator> GetKeyView() const { return { KeyIterator(m_buckets, 0), KeyIterator(m_buckets, m_buckets.Size()) }; }

	IteratorView<ValueIterator> GetValueView() { return { ValueIterator(m_buckets, 0), ValueIterator(m_buckets, m_buckets.Size()) }; }
	IteratorView<ConstValueIterator> GetValueView() const { return { ConstValueIterator(m_buckets, 0), ConstValueIterator(m_buckets, m_buckets.Size()) }; }

private:
	HashFn m_hashFunction;
	uint64_t m_shift;

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

// HashMap iterator implementations.
namespace Collection
{
#define HASH_MAP_ITERATOR_BASE(TYPENAME) \
public: \
	explicit TYPENAME(const Collection::Vector<Bucket>& buckets, size_t bucketIndex) \
		: m_buckets(const_cast<Collection::Vector<Bucket>*>(&buckets)) \
		, m_bucketIndex(bucketIndex) \
		, m_indexInBucket(0) \
	{ \
		while (m_bucketIndex < m_buckets->Size() && (*m_buckets)[m_bucketIndex].m_keys.IsEmpty()) \
		{ \
			++m_bucketIndex; \
		} \
	} \
\
	const TYPENAME& operator++() \
	{ \
		const auto& bucket = (*m_buckets)[m_bucketIndex]; \
		++m_indexInBucket; \
		if (m_indexInBucket >= bucket.m_keys.Size()) \
		{ \
			++m_bucketIndex; \
			while (m_bucketIndex < m_buckets->Size() && (*m_buckets)[m_bucketIndex].m_keys.IsEmpty()) \
			{ \
				++m_bucketIndex; \
			} \
			m_indexInBucket = 0; \
		} \
		return *this; \
	} \
\
	TYPENAME operator++(int) \
	{ \
		TYPENAME result = *this; \
		++(*this); \
		return result; \
	} \
\
	bool operator==(const TYPENAME& rhs) const \
	{ \
		AMP_FATAL_ASSERT(m_buckets == rhs.m_buckets, \
			"There is no defined comparison between iterators from different containers!"); \
		return m_bucketIndex == rhs.m_bucketIndex && m_indexInBucket == rhs.m_indexInBucket; \
	} \
\
	bool operator!=(const TYPENAME& rhs) const \
	{ \
		return !(*this == rhs); \
	} \
\
private: \
	Collection::Vector<Bucket>* m_buckets; \
	size_t m_bucketIndex; \
	size_t m_indexInBucket;

template <typename Bucket, typename KeyValuePair>
class HashMapKeyValueIterator final
{
	HASH_MAP_ITERATOR_BASE(HashMapKeyValueIterator)

public:
	using value_type = KeyValuePair;

	KeyValuePair operator*() const
	{
		const auto& bucket = (*m_buckets)[m_bucketIndex];
		return KeyValuePair(bucket.m_keys[m_indexInBucket], bucket.m_values[m_indexInBucket]);
	}
};

template <typename Bucket, typename KeyRef>
class HashMapKeyIterator final
{
	HASH_MAP_ITERATOR_BASE(HashMapKeyIterator)

public:
	using value_type = KeyRef;

	KeyRef operator*() const
	{
		const auto& bucket = (*m_buckets)[m_bucketIndex];
		return bucket.m_keys[m_indexInBucket];
	}
};

template <typename Bucket, typename Value>
class HashMapValueIterator final
{
	HASH_MAP_ITERATOR_BASE(HashMapValueIterator)

public:
	using value_type = Value;

	Value& operator*() const
	{
		auto& bucket = (*m_buckets)[m_bucketIndex];
		return bucket.m_values[m_indexInBucket];
	}
};

#undef HASH_MAP_ITERATOR_BASE
}

// Inline HashMap implementations.
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
