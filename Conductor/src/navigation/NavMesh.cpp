#include <navigation/NavMesh.h>

#include <limits>

namespace Navigation
{
static_assert(sizeof(NavMeshConnections) == 64,
	"Please ensure that NavMeshConnections align well to cache boundaries.");

size_t NavMesh::FindIndexOfID(const NavMeshTriangleID id) const
{
	return std::numeric_limits<size_t>::max();
}

NavMeshTriangle& NavMesh::GetTriangleByIndex(const size_t index)
{
	return m_trianglesByIDIndex[index];
}

const NavMeshTriangle& NavMesh::GetTriangleByIndex(const size_t index) const
{
	return m_trianglesByIDIndex[index];
}

NavMeshConnections& NavMesh::GetConnectionsByIndex(const size_t index)
{
	return m_connectionsByIDIndex[index];
}

const NavMeshConnections& NavMesh::GetConnectionsByIndex(const size_t index) const
{
	return m_connectionsByIDIndex[index];
}

Collection::Pair<size_t, NavMeshTriangleID> NavMesh::AddTriangle(const NavMeshTriangle& triangle)
{
	const NavMeshTriangleID triangleID = (m_triangleIDs.IsEmpty())
		? NavMeshTriangleID(0)
		: NavMeshTriangleID(m_triangleIDs.Back().GetValue() + 1);

	const size_t triangleIndex = m_triangleIDs.Size();

	m_triangleIDs.Add(triangleID);
	m_trianglesByIDIndex.Add(triangle);
	m_connectionsByIDIndex.Emplace();

	return { triangleIndex, triangleID };
}
}