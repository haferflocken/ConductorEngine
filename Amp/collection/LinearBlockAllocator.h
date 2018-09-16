#pragma once

#include <collection/Vector.h>

#include <cstdint>
#include <mutex>

namespace Collection
{
/**
 * An allocator that allows fixed size allocations. Implemented as a list of blocks of elements.
 */
class LinearBlockAllocator
{
public:
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
		, m_elementSizeInBytes(static_cast<uint32_t>(sizeInBytes))
	{}

	LinearBlockAllocator(LinearBlockAllocator&&);
	LinearBlockAllocator& operator=(LinearBlockAllocator&&);

	~LinearBlockAllocator();

	void* Alloc();
	void Free(void* ptr);

private:
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
}
