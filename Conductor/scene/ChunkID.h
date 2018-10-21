#pragma once

#include <cstdint>

namespace Scene
{
/**
 * A Chunk is a discrete portion of a scene which can be saved and loaded to disk.
 * A ChunkID stores the chunk's coordinates in a 3D grid of chunks.
 * There are 4 spare bits in each ChunkID which may be used by systems as needed.
 */
class ChunkID final
{
public:
	constexpr ChunkID()
		: m_data(k_invalidData)
	{}

	ChunkID(int32_t chunkX, int32_t chunkY, int32_t chunkZ, uint8_t extra = 0)
		: m_chunkX(chunkX)
		, m_chunkY(chunkY)
		, m_chunkZ(chunkZ)
		, m_extra(extra)
	{}

	int32_t GetX() const { return m_chunkX; }
	int32_t GetY() const { return m_chunkY; }
	int32_t GetZ() const { return m_chunkZ; }
	uint8_t GetExtra() const { return static_cast<uint8_t>(m_extra); }

	ChunkID GetWithoutExtra() const { return ChunkID(m_chunkX, m_chunkY, m_chunkZ); }

	bool operator==(const ChunkID& rhs) const { return m_data == rhs.m_data; }
	bool operator!=(const ChunkID& rhs) const { return m_data != rhs.m_data; }
	bool operator<(const ChunkID& rhs) const { return m_data < rhs.m_data; }

private:
	static constexpr uint64_t k_invalidData = UINT64_MAX;

	union
	{
		uint64_t m_data;
		struct
		{
			int32_t m_chunkX : 20;
			int32_t m_chunkY : 20;
			int32_t m_chunkZ : 20;
			uint32_t m_extra : 4;
		};
	};
};
}
