#pragma once

#include <cstdint>
#include <cstring>

namespace Collection
{
class BitVector
{
public:
	using value_type = bool;
	class iterator;
	class const_iterator;
	class BitReference;

	explicit BitVector(const uint32_t initialCapacity = 64);
	
	BitVector(const BitVector& o);
	void operator=(const BitVector& rhs);

	BitVector(BitVector&& o);
	void operator=(BitVector&& rhs);

	~BitVector();

	uint32_t Size() const { return m_count; }
	uint32_t Capacity() const { return m_capacity; }
	bool IsEmpty() const { return m_count == 0; }

	bool Front() const { return (*this)[0]; }
	bool Back() const { return (*this)[m_count - 1]; }

	BitReference operator[](const size_t i);
	bool operator[](const size_t i) const;

	iterator begin();
	const_iterator begin() const;
	const_iterator cbegin() const;

	iterator end();
	const_iterator end() const;
	const_iterator cend() const;

	void Resize(const uint32_t count, bool defaultValue);

	void Add(bool e);
	void Clear();

private:
	void EnsureCapacity(const uint32_t desiredCapacity);

	uint64_t* m_data{ nullptr };
	uint32_t m_capacity{ 0 };
	uint32_t m_count{ 0 };
};
}

// BitReference implementation.
namespace Collection
{
class BitVector::BitReference
{
	uint64_t* m_data;
	uint64_t m_mask;

public:
	BitReference(uint64_t* data, size_t bitIndex)
		: m_data(data)
		, m_mask(1Ui64 << bitIndex)
	{}

	operator bool() const
	{
		return ((*m_data) & m_mask) != 0;
	}

	void operator=(bool rhs)
	{
		if (rhs)
		{
			(*m_data) |= m_mask;
		}
		else
		{
			(*m_data) &= (~m_mask);
		}
	}
};
}

// Iterator implementations.
namespace Collection
{
class BitVector::const_iterator
{
protected:
	uint64_t* m_data;
	size_t m_bitIndex;

public:
	const_iterator(const uint64_t* data, size_t bitIndex)
		: m_data(const_cast<uint64_t*>(data))
		, m_bitIndex(bitIndex)
	{}
};

class BitVector::iterator : public BitVector::const_iterator
{
public:
	iterator(uint64_t* data, size_t bitIndex)
		: const_iterator(data, bitIndex)
	{}
};
}

// Inline method implementations.
namespace Collection
{
inline BitVector::BitVector(const uint32_t initialCapacity)
	: m_data(new uint64_t[(initialCapacity + 63) / 64])
	, m_capacity(((initialCapacity + 63) / 64) * 64) // Rounds up the capacity to what has been allocated.
	, m_count(0)
{}

inline BitVector::BitVector(const BitVector& o)
	: m_data(new uint64_t[(o.m_count + 63) / 64])
	, m_capacity(((o.m_count + 63) / 64) * 64) // Rounds up the capacity to what has been allocated.
	, m_count(o.m_count)
{}

inline void BitVector::operator=(const BitVector& rhs)
{
	delete[] m_data;
	const uint32_t wordCount = (rhs.m_count + 63) / 64;
	m_data = new uint64_t[wordCount];
	m_capacity = wordCount * 64; // Rounds up the capacity to what has been allocated.
	m_count = rhs.m_count;
}

inline BitVector::BitVector(BitVector&& o)
	: m_data(o.m_data)
	, m_capacity(o.m_capacity)
	, m_count(o.m_count)
{
	o.m_data = nullptr;
	o.m_capacity = 0;
	o.m_count = 0;
}

inline void BitVector::operator=(BitVector&& rhs)
{
	delete[] m_data;
	m_data = rhs.m_data;
	m_capacity = rhs.m_capacity;
	m_count = rhs.m_count;
	rhs.m_data = nullptr;
	rhs.m_capacity = 0;
	rhs.m_count = 0;
}

inline BitVector::~BitVector()
{
	delete[] m_data;
}

inline BitVector::BitReference BitVector::operator[](const size_t i)
{
	const size_t wordIndex = i / 64;
	const size_t bitIndex = i % 64;
	return BitReference(m_data + wordIndex, bitIndex);
}

inline bool BitVector::operator[](const size_t i) const
{
	const size_t wordIndex = i / 64;
	const size_t bitIndex = i % 64;
	return (m_data[wordIndex] & (1Ui64 << bitIndex)) != 0;
}

inline BitVector::iterator BitVector::begin()
{
	return iterator(m_data, 0);
}

inline BitVector::const_iterator BitVector::begin() const
{
	return const_iterator(m_data, 0);
}

inline BitVector::const_iterator BitVector::cbegin() const
{
	return begin();
}

inline BitVector::iterator BitVector::end()
{
	const size_t lastIndex = m_count - 1;
	const size_t wordIndex = lastIndex / 64;
	const size_t bitIndex = lastIndex % 64;
	return iterator(m_data + wordIndex, bitIndex);
}

inline BitVector::const_iterator BitVector::end() const
{
	const size_t lastIndex = m_count - 1;
	const size_t wordIndex = lastIndex / 64;
	const size_t bitIndex = lastIndex % 64;
	return const_iterator(m_data + wordIndex, bitIndex);
}

inline BitVector::const_iterator BitVector::cend() const
{
	return end();
}

inline void BitVector::Resize(const uint32_t count, bool defaultValue)
{
	EnsureCapacity(count);
	// TODO this could be more efficient by doing a memset when possible
	while (m_count < count)
	{
		Add(defaultValue);
	}
}

inline void BitVector::Add(bool e)
{
	const size_t i = m_count;
	EnsureCapacity(m_count);
	(*this)[i] = e;
	++m_count;
}

inline void BitVector::Clear()
{
	m_count = 0;
}

inline void BitVector::EnsureCapacity(const uint32_t desiredCapacity)
{
	if (desiredCapacity > Capacity())
	{
		// If we need more room, double our capacity as many times as we need to.
		uint32_t newCapacity = Capacity() * 2;
		while (newCapacity < desiredCapacity)
		{
			newCapacity *= 2;
		}
		uint64_t* const newData = new uint64_t[(newCapacity + 63) / 64];

		// Copy the contents of the old buffer into the new one.
		memcpy(newData, m_data, (m_count + 63) / 8);

		// Move the new buffer into m_data and delete the old buffer.
		delete[] m_data;
		m_data = newData;
		m_capacity = newCapacity;
	}
}
}
