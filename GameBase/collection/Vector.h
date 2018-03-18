#pragma once

#include <dev/Dev.h>
#include <traits/IsMemCopyAFullCopy.h>
#include <unit/CountUnits.h>

#include <algorithm>
#include <functional>
#include <initializer_list>

namespace Collection
{
template <typename T>
class Vector
{
public:
	using value_type = T;
	using iterator = T*;
	using const_iterator = const T*;

	static constexpr size_t sk_InvalidIndex = static_cast<size_t>(-1);

	explicit Vector(const uint32_t initialCapacity = 8);
	explicit Vector(std::initializer_list<T> initialElements);

	Vector(const Vector<T>& o);
	void operator=(const Vector<T>& rhs);

	Vector(Vector<T>&& o);
	void operator=(Vector<T>&& rhs);

	~Vector();

	uint32_t Size() const { return m_count; }
	uint32_t Capacity() const { return m_capacity; }
	bool IsEmpty() const { return m_count == 0; }

	T& Front() { return m_data[0]; }
	const T& Front() const { return m_data[0]; }

	T& Back() { return m_data[m_count - 1]; }
	const T& Back() const { return m_data[m_count - 1]; }

	T& operator[](const size_t i) { return m_data[i]; }
	const T& operator[](const size_t i) const { return m_data[i]; }

	iterator begin() { return m_data; }
	const_iterator begin() const { return m_data; }
	const_iterator cbegin() const { return begin(); }

	iterator end() { return begin() + m_count; }
	const_iterator end() const { return begin() + m_count; }
	const_iterator cend() const { return end(); }

	void Add(const T& e);
	void Add(T&& e);

	template <typename... Args>
	T& Emplace(Args&&... args);

	template <typename... Args>
	T& EmplaceAt(const size_t i, Args&&... args);

	void RemoveLast();
	void Remove(const size_t start, const size_t end);

	void Clear();

	size_t IndexOf(const std::function<bool(const T&)>& predicate) const;
	size_t IndexOf(const T& o) const { return IndexOf([&](const auto& e) { return o == e; }); }

	T* Find(const std::function<bool(const T&)>& predicate);
	const T* Find(const std::function<bool(const T&)>& predicate) const;

	// Partitions the vector based on a predicate and returns the index to the start of the second partition.
	// The partition for "true" values comes before the partition for "false" values.
	size_t Partition(std::function<bool(const T&)>&& fn);

private:
	void EnsureCapacity(const uint32_t desiredCapacity);

	T* m_data{ nullptr };
	uint32_t m_capacity{ 0 };
	uint32_t m_count{ 0 };
};

template <typename T>
inline Vector<T>::Vector(const uint32_t initialCapacity)
	: m_data(static_cast<T*>(malloc(initialCapacity * Unit::AlignedSizeOf<T>())))
	, m_capacity(initialCapacity)
	, m_count(0)
{}

template <typename T>
inline Vector<T>::Vector(std::initializer_list<T> initialElements)
	: m_data(static_cast<T*>(malloc(initialElements.size()* Unit::AlignedSizeOf<T>())))
	, m_capacity(static_cast<uint32_t>(initialElements.size()))
	, m_count(0)
{
	for (const auto& element : initialElements)
	{
		Add(element);
	}
}

template <typename T>
inline Vector<T>::Vector(const Vector<T>& o)
{
	*this = o;
}

template <typename T>
inline void Vector<T>::operator=(const Vector<T>& rhs)
{
	if (m_data != nullptr)
	{
		delete m_data;
	}

	m_data = static_cast<T*>(malloc(rhs.m_count * Unit::AlignedSizeOf<T>()));
	m_capacity = rhs.m_capacity;
	m_count = rhs.m_count;

	if (Traits::IsMemCopyAFullCopy<T>::value)
	{
		memcpy(m_data, rhs.m_data, m_count * Unit::AlignedSizeOf<T>());
	}
	else
	{
		for (size_t i = 0, iEnd = m_count; i < iEnd; ++i)
		{
			m_data[i] = rhs[i];
		}
	}
}

template <typename T>
inline Vector<T>::Vector(Vector<T>&& o)
	: m_data(o.m_data)
	, m_capacity(o.m_capacity)
	, m_count(o.m_count)
{
	o.m_data = nullptr;
	o.m_capacity = 0;
	o.m_count = 0;
}

template <typename T>
inline void Vector<T>::operator=(Vector<T>&& rhs)
{
	m_data = rhs.m_data;
	m_capacity = rhs.m_capacity;
	m_count = rhs.m_count;

	rhs.m_data = nullptr;
	rhs.m_capacity = 0;
	rhs.m_count = 0;
}

template <typename T>
inline Vector<T>::~Vector()
{
	for (T& element : *this)
	{
		(&element)->~T();
	}
}

template <typename T>
inline void Vector<T>::Add(const T& e)
{
	const uint32_t i = m_count;
	EnsureCapacity(i + 1);
	new (&m_data[i]) T(e);
	m_count += 1;
}

template <typename T>
inline void Vector<T>::Add(T&& e)
{
	const uint32_t i = m_count;
	EnsureCapacity(i + 1);
	new (&m_data[i]) T(std::forward<T>(e));
	m_count += 1;
}

template <typename T>
template <typename... Args>
inline T& Vector<T>::Emplace(Args&&... args)
{
	const uint32_t i = m_count;
	EnsureCapacity(i + 1);
	new (&m_data[i]) T(std::forward<Args>(args)...);
	m_count += 1;
	return m_data[i];
}

template <typename T>
template <typename... Args>
inline T& Vector<T>::EmplaceAt(const size_t i, Args&&... args)
{
	Dev::FatalAssert(i <= m_count, "Cannot emplace a new element outside of the range [0, size()].");

	// Increase the size by 1.
	const uint32_t lastIndex = m_count;
	EnsureCapacity(lastIndex + 1);
	m_count += 1;
	
	// Move the elements at and after i down by 1.
	// Move-construct the last element because it has no destination instance.
	if (lastIndex > 0)
	{
		if (lastIndex != i)
		{
			new (&m_data[lastIndex]) T(std::move(m_data[lastIndex - 1]));
		}

		for (size_t j = lastIndex; j > (i + 1); --j)
		{
			m_data[j - 1] = std::move(m_data[j - 2]);
		}
	}

	// Create and return the element at i.
	new (&m_data[i]) T(std::forward<Args>(args)...);
	return m_data[i];
}

template <typename T>
inline void Vector<T>::RemoveLast()
{
	(&m_data[m_count])->~T();
	--m_count;
}

template <typename T>
inline void Vector<T>::Remove(const size_t start, const size_t end)
{
	Dev::FatalAssert(start <= end, "Collection::Vector::Remove requires that start be less than or equal to end.");
	Dev::FatalAssert(end <= m_count, "Collection::Vector::Remove cannot remove past the end.");

	if (start == end)
	{
		return;
	}

	size_t i = 0;
	const size_t removeCount = (end - start);
	for (const size_t iEnd = std::min(removeCount, m_count - end); i < iEnd; ++i)
	{
		m_data[start + i] = std::move(m_data[end + i]);
	}
	i += start;
	for (const size_t iEnd = m_count; i < iEnd; ++i)
	{
		(&m_data[i])->~T();
	}
	m_count -= static_cast<uint32_t>(removeCount);
}

template <typename T>
inline void Vector<T>::Clear()
{
	Remove(0, m_count);
}

template <typename T>
inline size_t Vector<T>::IndexOf(const std::function<bool(const T&)>& predicate) const
{
	for (size_t i = 0, iEnd = m_count; i < iEnd; ++i)
	{
		if (predicate(m_data[i]))
		{
			return i;
		}
	}
	return sk_InvalidIndex;
}

template <typename T>
inline T* Vector<T>::Find(const std::function<bool(const T&)>& predicate)
{
	for (size_t i = 0, iEnd = m_count; i < iEnd; ++i)
	{
		if (predicate(m_data[i]))
		{
			return &m_data[i];
		}
	}
	return nullptr;
}

template <typename T>
inline const T* Vector<T>::Find(const std::function<bool(const T&)>& predicate) const
{
	for (size_t i = 0, iEnd = m_count; i < iEnd; ++i)
	{
		if (predicate(m_data[i]))
		{
			return &m_data[i];
		}
	}
	return nullptr;
}

template <typename T>
inline size_t Vector<T>::Partition(std::function<bool(const T&)>&& fn)
{
	const auto itr = std::partition(begin(), end(), std::move(fn));
	return std::distance(begin(), itr);
}

template <typename T>
inline void Vector<T>::EnsureCapacity(const uint32_t desiredCapacity)
{
	if (desiredCapacity > Capacity())
	{
		// If we need more room, double our capacity as many times as we need to.
		uint32_t newCapacity = Capacity() * 2;
		while (newCapacity < desiredCapacity)
		{
			newCapacity *= 2;
		}
		T* const newData = static_cast<T*>(malloc(newCapacity * Unit::AlignedSizeOf<T>()));

		// Move the contents of the old buffer into the new one.
		if (Traits::IsMemCopyAFullCopy<T>::value)
		{
			memcpy(newData, m_data, m_count * Unit::AlignedSizeOf<T>());
		}
		else
		{
			for (size_t i = 0, iEnd = m_count; i < iEnd; ++i)
			{
				new (&newData[i]) T(std::move(m_data[i]));
			}
		}

		// Move the new buffer into m_data and delete the old buffer.
		if (m_data != nullptr)
		{
			delete m_data;
		}
		m_data = newData;
		m_capacity = newCapacity;
	}
}
}
