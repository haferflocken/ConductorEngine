#pragma once

#include <util/UniqueID.h>

#include <cstdint>

namespace Navigation
{
/**
 * Uniquely identifies a triangle in a NavMesh.
 */
class NavMeshTriangleID : public Util::UniqueID<NavMeshTriangleID, uint32_t>
{
public:
	NavMeshTriangleID()
		: UniqueID()
	{}

	explicit NavMeshTriangleID(uint32_t value)
		: UniqueID(value)
	{}
};
}
