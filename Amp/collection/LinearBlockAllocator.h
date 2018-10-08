#pragma once

#include <collection/Vector.h>
#include <unit/CountUnits.h>

#include <cstdint>
#include <mutex>

namespace Collection
{
/**
 * An allocator that allows fixed size allocations. Implemented as a list of blocks of elements.
 * Allocations and frees are thread-safe by mutex. No other methods have guaranteed thread-safety.
 */
class LinearBlockAllocator
{
public:
	class iterator;
	friend class iterator;
	class const_iterator;
	friend class const_iterator;

	static constexpr size_t k_numElementsPerBlock = 64;

	template <typename T>
	static LinearBlockAllocator MakeFor()
	{
		return LinearBlockAllocator(alignof(T), sizeof(T));
	}

	LinearBlockAllocator() = default;

	LinearBlockAllocator(size_t alignmentInBytes, size_t sizeInBytes)
		: m_blocks()
		, m_mutex()
		, m_elementAlignmentInBytes(static_cast<uint32_t>(alignmentInBytes))
		, m_elementSizeInBytes(static_cast<uint32_t>(Unit::AlignedSizeOf(sizeInBytes, alignmentInBytes)))
	{}

	LinearBlockAllocator(LinearBlockAllocator&&);
	LinearBlockAllocator& operator=(LinearBlockAllocator&&);

	~LinearBlockAllocator();

	void* Alloc();
	void Free(void* ptr);

	bool IsEmpty() const;

	iterator begin();
	const_iterator begin() const;
	const_iterator cbegin() const;

	iterator end();
	const_iterator end() const;
	const_iterator cend() const;

private:
	uint8_t& Get(uint32_t blockIndex, uint32_t indexInBlock);
	const uint8_t& Get(uint32_t blockIndex, uint32_t indexInBlock) const;

	struct Block
	{
		uint64_t m_vacancyMap{ UINT64_MAX };
		void* m_memory{ nullptr };
	};
	Collection::Vector<Block> m_blocks;
	std::mutex m_mutex;
	uint32_t m_elementAlignmentInBytes{ 0 };
	uint32_t m_elementSizeInBytes{ 0 };
};

class LinearBlockAllocator::iterator
{
public:
	using difference_type = int64_t;
	using value_type = uint8_t;
	using pointer = uint8_t*;
	using reference = uint8_t&;
	using iterator_category = std::forward_iterator_tag;

	iterator()
		: m_allocator(nullptr)
		, m_blockIndex(0)
		, m_indexInBlock(0)
	{}

	iterator(LinearBlockAllocator& allocator, uint32_t blockIndex, uint32_t indexInBlock)
		: m_allocator(&allocator)
		, m_blockIndex(blockIndex)
		, m_indexInBlock(indexInBlock)
	{}

	reference operator*() const { return m_allocator->Get(m_blockIndex, m_indexInBlock); }
	pointer operator->() const { return &m_allocator->Get(m_blockIndex, m_indexInBlock); }

	bool operator==(const iterator& rhs) const { return memcmp(this, &rhs, sizeof(iterator)) == 0; }
	bool operator!=(const iterator& rhs) const { return !(*this == rhs); }

	iterator& operator++();
	iterator operator++(int) { iterator temp = *this; ++*this; return temp; }

private:
	LinearBlockAllocator* m_allocator;
	uint32_t m_blockIndex;
	uint32_t m_indexInBlock;
};

class LinearBlockAllocator::const_iterator
{
public:
	using difference_type = int64_t;
	using value_type = uint8_t;
	using pointer = const uint8_t*;
	using reference = const uint8_t&;
	using iterator_category = std::forward_iterator_tag;

	const_iterator()
		: m_allocator(nullptr)
		, m_blockIndex(0)
		, m_indexInBlock(0)
	{}

	const_iterator(const LinearBlockAllocator& allocator, uint32_t blockIndex, uint32_t indexInBlock)
		: m_allocator(&allocator)
		, m_blockIndex(blockIndex)
		, m_indexInBlock(indexInBlock)
	{}

	reference operator*() const { return m_allocator->Get(m_blockIndex, m_indexInBlock); }
	pointer operator->() const { return &m_allocator->Get(m_blockIndex, m_indexInBlock); }

	bool operator==(const const_iterator& rhs) const { return memcmp(this, &rhs, sizeof(const_iterator)) == 0; }
	bool operator!=(const const_iterator& rhs) const { return !(*this == rhs); }

	const_iterator& operator++();
	const_iterator operator++(int) { const_iterator temp = *this; ++*this; return temp; }

private:
	const LinearBlockAllocator* m_allocator;
	uint32_t m_blockIndex;
	uint32_t m_indexInBlock;
};
}

// Inline implementations: LinearBlockAllocator
namespace Collection
{
inline LinearBlockAllocator::iterator LinearBlockAllocator::begin()
{
	return iterator(*this, 0, 0);
}

inline LinearBlockAllocator::const_iterator LinearBlockAllocator::begin() const
{
	return const_iterator(*this, 0, 0);
}

inline LinearBlockAllocator::const_iterator LinearBlockAllocator::cbegin() const { return begin(); }

inline LinearBlockAllocator::iterator LinearBlockAllocator::end()
{
	return iterator(*this, m_blocks.Size(), 0);
}

inline LinearBlockAllocator::const_iterator LinearBlockAllocator::end() const
{
	return const_iterator(*this, m_blocks.Size(), 0);
}

inline LinearBlockAllocator::const_iterator LinearBlockAllocator::cend() const { return end(); }

inline uint8_t& LinearBlockAllocator::Get(uint32_t blockIndex, uint32_t indexInBlock)
{
	// Implemented in terms of the const variant.
	return const_cast<uint8_t&>(static_cast<const LinearBlockAllocator*>(this)->Get(blockIndex, indexInBlock));
}

inline const uint8_t& LinearBlockAllocator::Get(uint32_t blockIndex, uint32_t indexInBlock) const
{
	const uint8_t* const blockMem = reinterpret_cast<const uint8_t*>(m_blocks[blockIndex].m_memory);
	return *(blockMem + (m_elementSizeInBytes * indexInBlock));
}
}

// Inline implementations: LinearBlockAllocator::iterator and const_iterator
namespace Collection
{
inline LinearBlockAllocator::iterator& LinearBlockAllocator::iterator::operator++()
{
	const auto& blocks = m_allocator->m_blocks;
	
	// Increment to the next non-vacant index.
	do
	{
		++m_indexInBlock;
		if (m_indexInBlock >= 64)
		{
			++m_blockIndex;
			m_indexInBlock = 0;
		}
	} while (m_blockIndex < blocks.Size() && (blocks[m_blockIndex].m_vacancyMap & (1ui64 << m_indexInBlock)) != 0);

	return *this;
}

inline LinearBlockAllocator::const_iterator& LinearBlockAllocator::const_iterator::operator++()
{
	const auto& blocks = m_allocator->m_blocks;

	// Increment to the next non-vacant index.
	do
	{
		++m_indexInBlock;
		if (m_indexInBlock >= 64)
		{
			++m_blockIndex;
			m_indexInBlock = 0;
		}
	} while (m_blockIndex < blocks.Size() && (blocks[m_blockIndex].m_vacancyMap & (1ui64 << m_indexInBlock)) != 0);

	return *this;
}
}
