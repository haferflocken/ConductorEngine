#pragma once

#include <collection/Pair.h>
#include <Collection/Vector.h>

#include <cstdint>

namespace Navigation
{
class NavMeshTriangleID;
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

	size_t FindIndexOfID(const NavMeshTriangleID id) const;

	NavMeshTriangle& GetTriangleByIndex(const size_t index);
	const NavMeshTriangle& GetTriangleByIndex(const size_t index) const;

	NavMeshConnections& GetConnectionsByIndex(const size_t index);
	const NavMeshConnections& GetConnectionsByIndex(const size_t index) const;

	// Adds a triangle to the nav mesh and returns its index and ID.
	Collection::Pair<size_t, NavMeshTriangleID> AddTriangle(const NavMeshTriangle& triangle);
};

class NavMeshTriangleID
{
	static constexpr uint32_t k_invalidValue = UINT32_MAX;
	uint32_t m_value{ k_invalidValue };

public:
	NavMeshTriangleID() = default;

	explicit NavMeshTriangleID(uint32_t value)
		: m_value(value)
	{}

	uint32_t GetValue() const { return m_value; }

	bool operator==(const NavMeshTriangleID& rhs) const { return m_value == rhs.m_value; }
	bool operator!=(const NavMeshTriangleID& rhs) const { return m_value != rhs.m_value; }
	bool operator<(const NavMeshTriangleID& rhs) const { return m_value < rhs.m_value; }
	bool operator<=(const NavMeshTriangleID& rhs) const { return m_value <= rhs.m_value; }
	bool operator>(const NavMeshTriangleID& rhs) const { return m_value > rhs.m_value; }
	bool operator>=(const NavMeshTriangleID& rhs) const { return m_value >= rhs.m_value; }
};

struct NavMeshTriangle
{
	float m_x1, m_y1;
	float m_x2, m_y2;
	float m_x3, m_y3;
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
