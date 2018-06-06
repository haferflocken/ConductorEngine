#pragma once

#include <navigation/NavMesh.h>

namespace Navigation
{
/**
 * NavMeshGraphInterface satisfies AStarSearch's graph interface requirements on behalf of a NavMesh.
 */
class NavMeshGraphInterface
{
	const NavMesh& m_navMesh;

public:
	using NodeID = NavMeshTriangleID;
	using NodeConnection = NavMeshConnection;
	using CostType = float;

	NavMeshGraphInterface(const NavMesh& navMesh)
		: m_navMesh(navMesh)
	{}

	uint32_t NodeIDToIndex(const NavMeshTriangleID& nodeID) const;
	Collection::ArrayView<const NavMeshConnection> GetNeighbours(const NavMeshTriangleID& nodeID,
		const uint32_t nodeIndex) const;
	NavMeshTriangleID ConnectionToNodeID(const NavMeshConnection& connection) const;
	bool IsValidNeighbour(const NavMeshTriangleID& nodeID, const uint32_t nodeIndex,
		const NavMeshConnection& connection) const;
	float CalcCost(const NavMeshTriangleID& nodeID, const uint32_t nodeIndex,
		const NavMeshConnection& connection, const uint32_t connectedIndex) const;
	float Heuristic(const NavMeshTriangleID& nodeID, const uint32_t nodeIndex,
		const NavMeshTriangleID& goalID, const uint32_t goalIndex);
};
}

// Inline implementations.
namespace Navigation
{
inline uint32_t NavMeshGraphInterface::NodeIDToIndex(const NavMeshTriangleID& nodeID) const
{
	return static_cast<uint32_t>(m_navMesh.FindIndexOfID(nodeID));
}

inline Collection::ArrayView<const NavMeshConnection> NavMeshGraphInterface::GetNeighbours(
	const NavMeshTriangleID& nodeID,
	const uint32_t nodeIndex) const
{
	const NavMeshConnections& connections = m_navMesh.GetConnectionsByIndex(nodeIndex);
	return Collection::ArrayView<const NavMeshConnection>(connections.m_connections, connections.m_numConnections);
}

inline NavMeshTriangleID NavMeshGraphInterface::ConnectionToNodeID(const NavMeshConnection& connection) const
{
	return connection.m_connectedID;
}

inline bool NavMeshGraphInterface::IsValidNeighbour(
	const NavMeshTriangleID& nodeID,
	const uint32_t nodeIndex,
	const NavMeshConnection& connection) const
{
	return true;
}

inline float NavMeshGraphInterface::CalcCost(
	const NavMeshTriangleID& nodeID,
	const uint32_t nodeIndex,
	const NavMeshConnection& connection,
	const uint32_t connectedIndex) const
{
	const NavMeshTriangle& nodeTriangle = m_navMesh.GetTriangleByIndex(nodeIndex);
	const NavMeshTriangle& connectedTriangle = m_navMesh.GetTriangleByIndex(connectedIndex);
	const Math::Vector3 delta = nodeTriangle.m_center - connectedTriangle.m_center;
	return delta.Length();
}

inline float NavMeshGraphInterface::Heuristic(
	const NavMeshTriangleID& nodeID,
	const uint32_t nodeIndex,
	const NavMeshTriangleID& goalID,
	const uint32_t goalIndex)
{
	const NavMeshTriangle& nodeTriangle = m_navMesh.GetTriangleByIndex(nodeIndex);
	const NavMeshTriangle& goalTriangle = m_navMesh.GetTriangleByIndex(goalIndex);
	const Math::Vector3 delta = nodeTriangle.m_center - goalTriangle.m_center;
	return delta.Length();
}
}
