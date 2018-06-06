#pragma once

#include <collection/Pair.h>
#include <collection/Vector.h>
#include <math/Vector3.h>
#include <navigation/NavMeshTriangleID.h>
#include <util/UniqueID.h>

#include <cstdint>

namespace Navigation
{
struct NavMeshTriangle;
struct NavMeshConnection;
struct NavMeshConnections;

/**
 * A graph of triangles connected with navigation information.
 * The expected pattern for mutating the mesh is to call AddTriangle, GetTriangleByIndex,
 * and GetConnection on a non-const NavMesh.
 */
class NavMesh
{
	// A list of IDs of triangles in this navmesh.
	Collection::Vector<NavMeshTriangleID> m_triangleIDs{};
	// A list of triangles corresponding by index to their ID.
	Collection::Vector<NavMeshTriangle> m_trianglesByIDIndex{};
	// A list of connections corresponding by index to their ID.
	// Connections are directed; they are not guaranteed to be mutual.
	Collection::Vector<NavMeshConnections> m_connectionsByIDIndex{};

public:
	NavMesh() = default;

	NavMesh(NavMesh&& o);
	void NavMesh::operator=(NavMesh&& rhs);

	uint32_t FindIndexOfID(const NavMeshTriangleID id) const;

	NavMeshTriangle& GetTriangleByIndex(const uint32_t index);
	const NavMeshTriangle& GetTriangleByIndex(const uint32_t index) const;

	NavMeshConnections& GetConnectionsByIndex(const uint32_t index);
	const NavMeshConnections& GetConnectionsByIndex(const uint32_t index) const;

	// Adds a triangle to the nav mesh and returns its index and ID.
	Collection::Pair<uint32_t, NavMeshTriangleID> AddTriangle(const NavMeshTriangle& triangle);

	// Finds the first triangle containing the point and returns its index and ID.
	Collection::Pair<uint32_t, NavMeshTriangleID> FindTriangleContaining(const Math::Vector3& position) const;
};

struct NavMeshTriangle
{
	Math::Vector3 m_v1;
	Math::Vector3 m_v2;
	Math::Vector3 m_v3;
	Math::Vector3 m_center;
};

class NavMeshConnectionFlags
{
	uint16_t m_bits{ 0 };

public:
	NavMeshConnectionFlags() = default;
};

struct NavMeshConnection
{
	NavMeshTriangleID m_connectedID{};
	NavMeshConnectionFlags m_flags{};
	uint16_t m_width{ 0 };
};

struct NavMeshConnections
{
	// The maximum number of connections is chosen to help with cache line alignment.
	static constexpr size_t k_maxConnectionsPerTriangle = 7;

	uint64_t m_numConnections{ 0 };
	NavMeshConnection m_connections[k_maxConnectionsPerTriangle];
};
}
