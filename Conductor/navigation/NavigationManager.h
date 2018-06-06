#pragma once

#include <collection/VectorMap.h>
#include <navigation/Navigator.h>
#include <navigation/NavigatorID.h>
#include <navigation/NavMesh.h>
#include <navigation/PathCache.h>

namespace Navigation
{
/**
 * The top level layer of the Navigation API. Hides implementation details of the navigation
 * library from external sources.
 */
class NavigationManager
{
	Collection::VectorMap<NavigatorID, Navigator> m_navigatorMap{};
	NavigatorID m_nextNavigatorID{ 0 };
	NavMesh m_navMesh;
	PathCache<NavMeshTriangleID> m_pathCache{};

public:
	explicit NavigationManager(NavMesh&& navMesh);

	NavigatorID CreateNavigator(const Math::Vector3& position, const Math::Vector3& heading);
	void RemoveNavigator(const NavigatorID navigatorID);

	void SetGoalPosition(const NavigatorID navigatorID, const Math::Vector3& goalPosition);

	void Update();
};
}
