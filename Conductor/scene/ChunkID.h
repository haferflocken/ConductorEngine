#pragma once

#include <cstdint>

namespace Scene
{
/**
 * A Chunk is a discrete portion of a scene which can be saved and loaded to disk.
 * A ChunkID stores the chunk's coordinates in a 3D grid of chunks.
 * There are 16 spare bits in each ChunkID which may be used by systems as needed.
 */
class ChunkID final
{
public:
	constexpr ChunkID()
		: m_data(k_invalidData)
	{}

	ChunkID(int16_t chunkX, int16_t chunkY, int16_t chunkZ, int16_t extra = 0)
		: m_chunkX(chunkX)
		, m_chunkY(chunkY)
		, m_chunkZ(chunkZ)
		, m_extra(extra)
	{}

	int16_t GetX() const { return m_chunkX; }
	int16_t GetY() const { return m_chunkY; }
	int16_t GetZ() const { return m_chunkZ; }
	int16_t GetExtra() const { return m_extra; }

	ChunkID GetWithoutExtra() const { return ChunkID(m_chunkX, m_chunkY, m_chunkZ, 0); }

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
			int16_t m_chunkX;
			int16_t m_chunkY;
			int16_t m_chunkZ;
			int16_t m_extra;
		};
	};
};
}
