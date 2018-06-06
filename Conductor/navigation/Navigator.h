#pragma once

#include <math/Vector3.h>
#include <navigation/NavMeshTriangleID.h>

namespace Navigation
{
/**
 * Navigators are the Navigation library's representation of anything that moves
 * through the world deliberately (as opposed to, for example, a rock rolling due to physics).
 */
struct Navigator
{
	Math::Vector3 m_position{};
	Math::Vector3 m_heading{ 1.0f, 0.0f, 0.0f };
	float m_speed{ 0.0f };
	Math::Vector3 m_goalPosition{};
	NavMeshTriangleID m_currentTriangle{};
	NavMeshTriangleID m_goalTriangle{};
};
}
