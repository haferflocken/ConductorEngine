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

namespace Internal_NavigationManager
{
void AdvanceWaypointForNavigator(Navigator& navigator)
{
	// TODO
}
}

void NavigationManager::Update()
{
	// Pathfind for navigators that need it.
	// For navigators that already have paths, advance their waypoints if they have reached them.
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
			// If the navigator's path has only one node, it cannot be followed but should not be recalculated.
			if (path.Size() == 1)
			{
				continue;
			}

			// Advance the navigator's waypoint if the waypoint is within the navigator's radius.
			const Math::Vector3 positionToWaypoint = navigator.m_waypointPosition - navigator.m_position;
			const float waypointProximitySquared = positionToWaypoint.LengthSquared();
			if (waypointProximitySquared <= (navigator.m_radius * navigator.m_radius))
			{
				Internal_NavigationManager::AdvanceWaypointForNavigator(navigator);
			}
			continue;
		}

		// If there is no path, pathfind.
		const bool pathFound = AStarSearch(graphInterface, navigator.m_currentTriangle, navigator.m_goalTriangle,
			MakePathFromNodes<NavMeshTriangleID, float>{ path });

		if (pathFound)
		{
			// If pathfinding succeeds, advance the navigator's waypoint.
			Internal_NavigationManager::AdvanceWaypointForNavigator(navigator);
		}
		else
		{
			// If pathfinding fails, add the current node to the path to indicate no path is needed.
			// Set the waypoint position to the navigator's current position because it has nowhere to go.
			path.Add(navigator.m_currentTriangle);
			navigator.m_waypointPosition = navigator.m_position;
		}
	}

	// Guide all navigators to their next waypoint.
	for (auto& entry : m_navigatorMap)
	{
		Navigator& navigator = entry.second;
		if (navigator.m_waypointPosition == navigator.m_goalPosition)
		{
			GuideNavigatorToGoal(navigator);
		}
		else
		{
			GuideNavigatorToWaypoint(navigator);
		}
	}
}

namespace Internal_NavigationManager
{
void GuideNavigatorToTarget(
	Navigator& navigator,
	const float targetProximity,
	const Math::Vector3& normalizedPositionToTarget)
{
	const float speed = navigator.m_speed;
	const float maxSpeed = navigator.m_maxSpeed;
	const float maxAcceleration = navigator.m_maxAcceleration;
	float newSpeed;
	if (speed < targetProximity)
	{
		// Go as fast as possible towards the target without overshooting it.
		const float maxNewSpeed = ((speed + maxAcceleration) < maxSpeed) ? (speed + maxAcceleration) : maxSpeed;
		newSpeed = (maxNewSpeed < targetProximity) ? maxNewSpeed : targetProximity;
	}
	else
	{
		// Slow down towards the target.
		const float deceleration = (targetProximity < maxAcceleration) ? targetProximity : maxAcceleration;
		newSpeed = (deceleration < speed) ? (speed - deceleration) : 0.0f;
	}

	navigator.m_position += (normalizedPositionToTarget * newSpeed);
	navigator.m_heading = normalizedPositionToTarget;
	navigator.m_speed = newSpeed;
}
}

void NavigationManager::GuideNavigatorToWaypoint(Navigator& navigator)
{
	const Math::Vector3 positionToWaypoint = navigator.m_waypointPosition - navigator.m_position;
	const float waypointProximitySquared = positionToWaypoint.LengthSquared();

	// If the navigator is within its radius of the waypoint, it doesn't need to get closer.
	if (waypointProximitySquared <= (navigator.m_radius * navigator.m_radius))
	{
		return;
	}

	const float waypointProximity = sqrtf(waypointProximitySquared);
	const Math::Vector3 normalizedPositionToWaypoint = positionToWaypoint / waypointProximity;

	Internal_NavigationManager::GuideNavigatorToTarget(navigator, waypointProximity, normalizedPositionToWaypoint);
}

void NavigationManager::GuideNavigatorToGoal(Navigator& navigator)
{
	const Math::Vector3 positionToGoal = navigator.m_goalPosition - navigator.m_position;
	const float goalProximitySquared = positionToGoal.LengthSquared();

	// If the navigator is within its required proximity of the goal, it doesn't need to get closer.
	if (goalProximitySquared <= (navigator.m_requiredProximity * navigator.m_requiredProximity))
	{
		return;
	}

	const float goalProximity = sqrtf(goalProximitySquared);
	const Math::Vector3 normalizedPositionToGoal = positionToGoal / goalProximity;

	Internal_NavigationManager::GuideNavigatorToTarget(navigator, goalProximity, normalizedPositionToGoal);
}
}
