#include <navigation/NavigationManager.h>

#include <dev/Dev.h>
#include <navigation/AStar.h>
#include <navigation/NavMeshGraphInterface.h>

namespace Navigation
{
NavigationManager::NavigationManager(NavMesh&& navMesh)
	: m_navMesh(std::move(navMesh))
{}

NavigatorID NavigationManager::CreateNavigator(const Math::Vector3& position, const Math::Vector3& heading)
{
	const NavigatorID navigatorID = m_nextNavigatorID;
	m_nextNavigatorID = NavigatorID(m_nextNavigatorID.GetUniqueID() + 1);

	Navigator& navigator = m_navigatorMap[navigatorID];
	navigator.m_position = position;
	navigator.m_heading = heading;
	navigator.m_goalPosition = position;
	
	const NavMeshTriangleID triangleID = m_navMesh.FindTriangleContaining(position).second;
	navigator.m_currentTriangle = triangleID;
	navigator.m_goalTriangle = triangleID;

	return navigatorID;
}

void NavigationManager::RemoveNavigator(const NavigatorID navigatorID)
{
	const bool success = m_navigatorMap.TryRemove(navigatorID);
	Dev::Assert(success, "Failed to find a navigator with ID [%u].", navigatorID.GetUniqueID());
}

void NavigationManager::SetGoalPosition(const NavigatorID navigatorID, const Math::Vector3& goalPosition)
{
	Navigator& navigator = m_navigatorMap[navigatorID];
	navigator.m_goalPosition = goalPosition;
	navigator.m_goalTriangle = m_navMesh.FindTriangleContaining(goalPosition).second;
}

void NavigationManager::Update()
{
	// Pathfind for navigators that need it.
	NavMeshGraphInterface graphInterface{ m_navMesh };
	for (auto& entry : m_navigatorMap)
	{
		Navigator& navigator = entry.second;
		if (navigator.m_currentTriangle == navigator.m_goalTriangle)
		{
			continue;
		}
		const auto* const cachedPath = m_pathCache.FindPath(navigator.m_currentTriangle, navigator.m_goalTriangle);
		if (cachedPath != nullptr)
		{
			continue;
		}

		// TODO there is a way to cache all the sub paths
		Collection::Vector<NavMeshTriangleID> path;
		if (AStarSearch(graphInterface, navigator.m_currentTriangle, navigator.m_goalTriangle, path))
		{

		}

		// Always add a path to the cache, even if it is empty.
		// This prevents repeatedly searching for a path that will never be found.
		m_pathCache.AddPath(navigator.m_currentTriangle, navigator.m_goalTriangle, std::move(path));
	}

	// TODO Guidance for all navigators not at their goal position
}
}
