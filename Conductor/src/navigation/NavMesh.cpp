#include <navigation/NavMesh.h>

#include <limits>

namespace Navigation
{
static_assert(sizeof(NavMeshConnections) == 64,
	"Please ensure that NavMeshConnections align well to cache boundaries.");

NavMesh::NavMesh(NavMesh&& o) noexcept
	: m_triangleIDs(std::move(o.m_triangleIDs))
	, m_trianglesByIDIndex(std::move(o.m_trianglesByIDIndex))
	, m_connectionsByIDIndex(std::move(o.m_connectionsByIDIndex))
{}

void NavMesh::operator=(NavMesh&& rhs) noexcept
{
	m_triangleIDs = std::move(rhs.m_triangleIDs);
	m_trianglesByIDIndex = std::move(rhs.m_trianglesByIDIndex);
	m_connectionsByIDIndex = std::move(rhs.m_connectionsByIDIndex);
}

uint32_t NavMesh::FindIndexOfID(const NavMeshTriangleID id) const
{
	for (auto iter = m_triangleIDs.begin(), iterEnd = m_triangleIDs.end(); iter != iterEnd; ++iter)
	{
		if (*iter == id)
		{
			return static_cast<uint32_t>(std::distance(m_triangleIDs.begin(), iter));
		}
	}
	return std::numeric_limits<uint32_t>::max();
}

NavMeshTriangle& NavMesh::GetTriangleByIndex(const uint32_t index)
{
	return m_trianglesByIDIndex[index];
}

const NavMeshTriangle& NavMesh::GetTriangleByIndex(const uint32_t index) const
{
	return m_trianglesByIDIndex[index];
}

NavMeshConnections& NavMesh::GetConnectionsByIndex(const uint32_t index)
{
	return m_connectionsByIDIndex[index];
}

const NavMeshConnections& NavMesh::GetConnectionsByIndex(const uint32_t index) const
{
	return m_connectionsByIDIndex[index];
}

Collection::Pair<uint32_t, NavMeshTriangleID> NavMesh::AddTriangle(const NavMeshTriangle& triangle)
{
	const NavMeshTriangleID triangleID = (m_triangleIDs.IsEmpty())
		? NavMeshTriangleID(0)
		: NavMeshTriangleID(m_triangleIDs.Back().GetUniqueID() + 1);

	const uint32_t triangleIndex = m_triangleIDs.Size();

	m_triangleIDs.Add(triangleID);
	m_trianglesByIDIndex.Add(triangle);
	m_connectionsByIDIndex.Emplace();

	return { triangleIndex, triangleID };
}

Collection::Pair<uint32_t, NavMeshTriangleID> NavMesh::FindTriangleContaining(const Math::Vector3& position) const
{
	// TODO Some sort of spatial paritioning for triangles
	for (auto iter = m_trianglesByIDIndex.begin(), iterEnd = m_trianglesByIDIndex.end(); iter != iterEnd; ++iter)
	{
		const NavMeshTriangle& triangle = *iter;
		// TODO test if triangle contains
	}
	return { UINT32_MAX, NavMeshTriangleID() };
}
}