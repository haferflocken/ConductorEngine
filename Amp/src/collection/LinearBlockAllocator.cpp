#include <collection/LinearBlockAllocator.h>

#include <dev/Dev.h>

namespace Collection
{
LinearBlockAllocator::LinearBlockAllocator(LinearBlockAllocator&& other)
{
	std::unique_lock lock{ other.m_mutex };

	m_blocks = std::move(other.m_blocks);
	m_elementAlignmentInBytes = other.m_elementAlignmentInBytes;
	m_elementSizeInBytes = other.m_elementSizeInBytes;
}

LinearBlockAllocator& LinearBlockAllocator::operator=(LinearBlockAllocator&& rhs)
{
	std::unique_lock lhsLock{ m_mutex, std::defer_lock };
	std::unique_lock rhsLock{ rhs.m_mutex, std::defer_lock };
	std::lock(lhsLock, rhsLock);

	m_blocks = std::move(rhs.m_blocks);
	m_elementAlignmentInBytes = rhs.m_elementAlignmentInBytes;
	m_elementSizeInBytes = rhs.m_elementSizeInBytes;

	return *this;
}

LinearBlockAllocator::~LinearBlockAllocator()
{
	Dev::Assert(m_blocks.IsEmpty(),
		"LinearAllocators should not be destroyed until everything allocated from them is freed.");
}

void* LinearBlockAllocator::Alloc()
{
	std::lock_guard guard{ m_mutex };

	if ((!m_blocks.IsEmpty()) && m_blocks.Back().m_vacancyMap != 0)
	{
		Block& block = m_blocks.Back();

		unsigned long index;
		_BitScanReverse64(&index, block.m_vacancyMap);

		uint8_t* result = reinterpret_cast<uint8_t*>(block.m_memory) + (index * m_elementSizeInBytes);
		block.m_vacancyMap ^= (1ui64 << index);
		return result;
	}

	Block& block = m_blocks.Emplace();
	block.m_vacancyMap ^= 1;
	block.m_memory = _aligned_malloc(m_elementSizeInBytes * k_numElementsPerBlock, m_elementAlignmentInBytes);

	return block.m_memory;
}

void LinearBlockAllocator::Free(void* ptr)
{
	std::lock_guard guard{ m_mutex };

	for (size_t i = 0, iEnd = m_blocks.Size(); i < iEnd; ++i)
	{
		Block& block = m_blocks[i];
		const uint8_t* const blockMemoryBegin = reinterpret_cast<const uint8_t*>(block.m_memory);
		const uint8_t* const blockMemoryEnd = blockMemoryBegin + (m_elementSizeInBytes * k_numElementsPerBlock);

		if (ptr >= block.m_memory && ptr < blockMemoryEnd)
		{
			const std::ptrdiff_t indexInBlock =
				(reinterpret_cast<const uint8_t*>(ptr) - blockMemoryBegin) / m_elementSizeInBytes;

			Dev::FatalAssert((block.m_vacancyMap & (1ui64 << indexInBlock)) == 0,
				"Cannot free an already freed pointer in LinearAllocator.");

			block.m_vacancyMap |= (1ui64 << indexInBlock);

			if (block.m_vacancyMap == UINT64_MAX)
			{
				_aligned_free(block.m_memory);
				m_blocks.SwapWithAndRemoveLast(i);
			}
			return;
		}
	}
	Dev::FatalError("LinearAllocator failed to free pointer %p.", ptr);
}
}
