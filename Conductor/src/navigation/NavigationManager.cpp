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

		Collection::Vector<NavMeshTriangleID>& path = m_pathsByNavigatorID[entry.first];
		if (!path.IsEmpty())
		{
			continue;
		}

		// If there is no path, pathfind.
		const bool pathFound = AStarSearch(graphInterface, navigator.m_currentTriangle, navigator.m_goalTriangle,
			MakePathFromNodes<NavMeshTriangleID, float>{ path });

		// If pathfinding fails, add the current node to the path to indicate no path is needed.
		if (!pathFound)
		{
			path.Add(navigator.m_currentTriangle);
		}
	}

	// Guide all navigators not within their required proximity to their goal position.
	for (auto& entry : m_navigatorMap)
	{
		GuideNavigator(entry.first, entry.second);
	}
}

void NavigationManager::GuideNavigator(const NavigatorID& navigatorID, Navigator& navigator)
{
	const Math::Vector3 positionToGoal = navigator.m_goalPosition - navigator.m_position;
	const float goalProximitySquared = positionToGoal.LengthSquared();
	if (goalProximitySquared <= (navigator.m_requiredProximity - navigator.m_requiredProximity))
	{
		return;
	}

	const float goalProximity = sqrtf(goalProximitySquared);
	const Math::Vector3 normalizedPositionToGoal = positionToGoal / goalProximity;

	const float speed = navigator.m_speed;
	const float maxSpeed = navigator.m_maxSpeed;
	const float maxAcceleration = navigator.m_maxAcceleration;
	float newSpeed;
	if (speed < goalProximity)
	{
		// Go as fast as possible towards the goal without overshooting it.
		const float maxNewSpeed = ((speed + maxAcceleration) < maxSpeed) ? (speed + maxAcceleration) : maxSpeed;
		newSpeed = (maxNewSpeed < goalProximity) ? maxNewSpeed : goalProximity;
	}
	else
	{
		// Slow down towards the goal.
		const float deceleration = (goalProximity < maxAcceleration) ? goalProximity : maxAcceleration;
		newSpeed = (deceleration < speed) ? (speed - deceleration) : 0.0f;
	}
	
	navigator.m_position += (normalizedPositionToGoal * newSpeed);
	navigator.m_heading = normalizedPositionToGoal;
	navigator.m_speed = newSpeed;
}
}
