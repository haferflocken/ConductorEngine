#pragma once

#include <collection/BitVector.h>
#include <collection/Heap.h>

#include <limits>

namespace Navigation
{
template <typename NodeID, typename CostType>
struct AStarNode;

template <typename NodeID, typename CostType>
struct MakePathFromNodes
{
	Collection::Vector<NodeID>& m_outPath;

	void OnPathFound(const Collection::Vector<AStarNode<NodeID, CostType>>& nodes, const uint32_t goalNodeIndex);
};

namespace AStarDetail
{
template <typename NodeID, typename CostType>
struct AStarHeapProperty
{
	Collection::Vector<AStarNode<NodeID, CostType>>* m_nodes;

	bool Test(const uint32_t& parent, const uint32_t& child) const;
};
}

/**
 * A general purpose A* implementation. Assumes that the heuristic is monotone.
 * The provided graph interface must define all of the following types:
 * - NodeID
 * - NodeConnection
 * - CostType
 *
 * The provided graph interface must have all of the following member functions:
 * - uint32_t NodeIDToIndex(const NodeID& nodeID) const
 * - Collection::ArrayView<const NodeConnection> GetNeighbours(const NodeID& nodeID, const uint32_t nodeIndex) const
 * - NodeID ConnectionToNodeID(const NodeConnection& connection) const
 * - bool IsValidNeighbour(const NodeID& nodeID, const uint32_t nodeIndex, const NodeConnection& connection) const
 * - CostType CalcCost(const NodeID& nodeID, const uint32_t nodeIndex,
 *       const NodeConnection& connection, const uint32_t connectedIndex) const
 * - CostType Heuristic(const NodeID& nodeID, const uint32_t nodeIndex, const NodeID& goalID, const uint32_t goalIndex)
 *
 * The provided output interface must have the following member function:
 * - void OnPathFound(const Collection::Vector<AStarNode<NodeID, CostType>>& nodes, const uint32_t goalNodeIndex)
 *
 * If a path is found, calls OnPathFound on outputInterface and returns true.
 */
template <typename GraphInterfaceType, typename OutputInterfaceType>
bool AStarSearch(
	GraphInterfaceType& graphInterface,
	const typename GraphInterfaceType::NodeID& startNodeID,
	const typename GraphInterfaceType::NodeID& goalNodeID,
	OutputInterfaceType& outputInterface)
{
	using namespace AStarDetail;
	using NodeID = typename GraphInterfaceType::NodeID;
	using NodeConnection = typename GraphInterfaceType::NodeConnection;
	using CostType = typename  GraphInterfaceType::CostType;
	using HeapProperty = AStarHeapProperty<NodeID, CostType>;

	const uint32_t goalNodeIndex = graphInterface.NodeIDToIndex(goalNodeID);

	Collection::Vector<AStarNode<NodeID, CostType>> nodes;
	Collection::Heap<uint32_t, 2, HeapProperty> openQueue{ HeapProperty{ &nodes } };
	Collection::BitVector closedSet;
	{
		const uint32_t startNodeIndex = graphInterface.NodeIDToIndex(startNodeID);
		const uint32_t higherIndex = (startNodeIndex > goalNodeIndex) ? startNodeIndex : goalNodeIndex;
		
		nodes.Resize(higherIndex + 1);
		AStarNode<NodeID, CostType>& startNode = nodes[startNodeIndex];
		startNode.m_nodeID = startNodeID;
		startNode.m_costFromStart = 0;
		startNode.m_estimatedCostFromStartToGoal = graphInterface.Heuristic(
			startNodeID, startNodeIndex, goalNodeID, goalNodeIndex);

		openQueue.Add(startNodeIndex);

		closedSet.Resize(higherIndex + 1, false);
	}
	
	while (!openQueue.IsEmpty())
	{
		// Pop the node at the front of the queue.
		const uint32_t currentIndex = openQueue.Pop();
		const AStarNode<NodeID, CostType>& current = nodes[currentIndex];
		if (current.m_nodeID == goalNodeID)
		{
			outputInterface.OnPathFound(nodes, goalNodeIndex);
			return true;
		}

		// Mark the current node as closed.
		if (currentIndex > closedSet.Size())
		{
			closedSet.Resize(currentIndex + 1, false);
		}
		closedSet[currentIndex] = true;

		// Evaluate any neighbours which are not closed.
		for (const auto& neighbourConnection : graphInterface.GetNeighbours(current.m_nodeID, currentIndex))
		{
			const NodeID neighbourNodeID = graphInterface.ConnectionToNodeID(neighbourConnection);
			const uint32_t neighbourIndex = graphInterface.NodeIDToIndex(neighbourNodeID);
			if ((neighbourIndex < closedSet.Size() && closedSet[neighbourIndex])
				|| !graphInterface.IsValidNeighbour(current.m_nodeID, currentIndex, neighbourConnection))
			{
				continue;
			}

			if (neighbourIndex > nodes.Size())
			{
				nodes.Resize(neighbourIndex + 1);
			}
			AStarNode<NodeID, CostType>& neighbour = nodes[neighbourIndex];

			const CostType stepCost = graphInterface.CalcCost(
				current.m_nodeID, currentIndex, neighbourConnection, neighbourIndex);
			const CostType candidateCost = current.m_costFromStart + stepCost;
			if ((!neighbour.HasCosts()) || neighbour.m_costFromStart > candidateCost)
			{
				// This doesn't cache the heuristic value because the heuristic could be adaptive.
				const CostType estimatedCostToGoal = graphInterface.Heuristic(
					neighbourNodeID, neighbourIndex, goalNodeID, goalNodeIndex);

				if (!neighbour.HasCosts())
				{
					neighbour.m_parentNodeIndex = currentIndex;
					neighbour.m_nodeID = neighbourNodeID;
					neighbour.m_costFromStart = candidateCost;
					neighbour.m_estimatedCostFromStartToGoal = candidateCost + estimatedCostToGoal;
					openQueue.Add(neighbourIndex);
				}
				else
				{
					neighbour.m_parentNodeIndex = currentIndex;
					neighbour.m_nodeID = neighbourNodeID;
					neighbour.m_costFromStart = candidateCost;
					neighbour.m_estimatedCostFromStartToGoal = candidateCost + estimatedCostToGoal;

					// Update the neighbour in the heap.
					for (auto iter = openQueue.begin(), iterEnd = openQueue.end(); iter != iterEnd; ++iter)
					{
						if ((*iter) == neighbourIndex)
						{
							openQueue.NotifyElementChanged(iter);
							break;
						}
					}
				}
			}
		}
	}

	return false;
}
}

namespace Navigation
{
template <typename NodeID, typename CostType>
struct AStarNode
{
	size_t m_parentNodeIndex{ std::numeric_limits<size_t>::max() };
	NodeID m_nodeID;
	CostType m_costFromStart; // Often known as the g-value.
	CostType m_estimatedCostFromStartToGoal; // Often known as the f-value.

	bool HasCosts() const { return m_parentNodeIndex != std::numeric_limits<size_t>::max(); }
};

template <typename NodeID, typename CostType>
void MakePathFromNodes<NodeID, CostType>::OnPathFound(
	const Collection::Vector<AStarNode<NodeID, CostType>>& nodes,
	const uint32_t goalNodeIndex)
{
	size_t index = goalNodeIndex;
	do
	{
		const AStarNode<NodeID, CostType>& current = nodes[index];
		m_outPath.Add(current.m_nodeID);
		index = current.m_parentNodeIndex;
	} while (index != std::numeric_limits<size_t>::max());
}
}

namespace Navigation::AStarDetail
{
template <typename NodeID, typename CostType>
bool AStarHeapProperty<NodeID, CostType>::Test(const uint32_t& parent, const uint32_t& child) const
{
	return (*m_nodes)[parent].m_estimatedCostFromStartToGoal < (*m_nodes)[child].m_estimatedCostFromStartToGoal;
}
}
