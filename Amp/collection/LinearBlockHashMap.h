#pragma once

#include <collection/HashMap.h>
#include <collection/LinearBlockAllocator.h>

#include <dev/Dev.h>

namespace Collection
{
/**
 * A key-value map with constant (amortized) access.
 * Values are allocated in a LinearBlockAllocator and therefore are guaranteed not to move during their lifetime.
 */
template <typename KeyType, typename ValueType, typename HashFn>
class LinearBlockHashMap
{
public:
	LinearBlockHashMap(HashFn&& hashFn, uint32_t numBucketsShift)
		: m_allocator(Collection::LinearBlockAllocator::MakeFor<ValueType>())
		, m_hashMap(std::move(hashFn), numBucketsShift)
	{}

	~LinearBlockHashMap();

	LinearBlockHashMap(const LinearBlockHashMap&) = delete;
	LinearBlockHashMap& operator=(const LinearBlockHashMap&) = delete;

	ValueType* Find(const KeyType& key);
	const ValueType* Find(const KeyType& key) const;

	// Construct a value in this map. If there is already a value at this key, this will fatal error.
	template <typename... Args>
	ValueType& Emplace(const KeyType& key, Args&&... args);
	template <typename... Args>
	ValueType& Emplace(KeyType&& key, Args&&... args);

	void Clear();

	bool TryRemove(const KeyType& key, ValueType* outRemovedValue = nullptr);

	bool IsEmpty() const { return m_allocator.IsEmpty(); }

private:
	LinearBlockAllocator m_allocator;
	HashMap<KeyType, ValueType*, HashFn> m_hashMap;
};
}

// Inline implementations.
namespace Collection
{
template <typename KeyType, typename ValueType, typename HashFn>
inline LinearBlockHashMap<KeyType, ValueType, HashFn>::~LinearBlockHashMap()
{
	Clear();
}

template <typename KeyType, typename ValueType, typename HashFn>
inline ValueType* LinearBlockHashMap<KeyType, ValueType, HashFn>::Find(const KeyType& key)
{
	const auto iter = m_hashMap.Find(key);
	return (iter != nullptr) ? *iter : nullptr;
}

template <typename KeyType, typename ValueType, typename HashFn>
inline const ValueType* LinearBlockHashMap<KeyType, ValueType, HashFn>::Find(const KeyType& key) const
{
	const auto iter = m_hashMap.Find(key);
	return (iter != nullptr) ? *iter : nullptr;
}

template <typename KeyType, typename ValueType, typename HashFn>
template <typename... Args>
inline ValueType& LinearBlockHashMap<KeyType, ValueType, HashFn>::Emplace(const KeyType& key, Args&&... args)
{
	AMP_FATAL_ASSERT(m_hashMap.Find(key) == nullptr, "Cannot emplace a value at an occupied key.");

	ValueType* const val = reinterpret_cast<ValueType*>(m_allocator.Alloc());
	new(val) ValueType(std::forward<Args>(args)...);
	m_hashMap[key] = val;
	return *val;
}

template <typename KeyType, typename ValueType, typename HashFn>
template <typename... Args>
inline ValueType& LinearBlockHashMap<KeyType, ValueType, HashFn>::Emplace(KeyType&& key, Args&&... args)
{
	AMP_FATAL_ASSERT(m_hashMap.Find(key) == nullptr, "Cannot emplace a value at an occupied key.");

	ValueType* const val = reinterpret_cast<ValueType*>(m_allocator.Alloc());
	new(val) ValueType(std::forward<Args>(args)...);
	m_hashMap[std::move(key)] = val;
	return *val;
}

template <typename KeyType, typename ValueType, typename HashFn>
inline void LinearBlockHashMap<KeyType, ValueType, HashFn>::Clear()
{
	for (size_t i = 0, n = m_hashMap.GetNumBuckets(); i < n; ++i)
	{
		for (auto&& ptr : m_hashMap.GetBucketViewAt(i).m_values)
		{
			ptr->~ValueType();
			m_allocator.Free(ptr);
		}
	}
}

template <typename KeyType, typename ValueType, typename HashFn>
inline bool LinearBlockHashMap<KeyType, ValueType, HashFn>::TryRemove(const KeyType& key, ValueType* outRemovedValue)
{
	ValueType* removedPtr = nullptr;
	if (m_hashMap.TryRemove(key, &removedPtr))
	{
		if (outRemovedValue != nullptr)
		{
			*outRemovedValue = std::move(*removedPtr);
		}
		removedPtr->~ValueType();
		m_allocator.Free(removedPtr);
		return true;
	}
	return false;
}
}
