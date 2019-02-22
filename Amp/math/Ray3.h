#pragma once

#include <math/Vector3.h>

namespace Math
{
/**
 * Ray3 represents an infinitely long line in 3D that extends from a point (m_origin) in a direction (m_direction).
 */
class Ray3
{
public:
	// Max value constructor.
	Ray3() = default;

	Ray3(const Math::Vector3& origin, const Math::Vector3& direction);

	bool TryCalcIntersectionWithPlane(const Math::Vector3& planeNormal,
		const Math::Vector3& pointOnPlane,
		Math::Vector3& outIntersection) const;

	Math::Vector3 m_origin;
	Math::Vector3 m_direction;
};
}

// Inline implementations.
namespace Math
{
inline Ray3::Ray3(const Math::Vector3& origin, const Math::Vector3& direction)
	: m_origin(origin)
	, m_direction(direction)
{}

inline bool Ray3::TryCalcIntersectionWithPlane(const Math::Vector3& planeNormal,
	const Math::Vector3& pointOnPlane,
	Math::Vector3& outIntersection) const
{
	const float denominator = planeNormal.Dot(m_direction);
	if (denominator > FLT_EPSILON)
	{
		const float numerator = planeNormal.Dot(pointOnPlane - m_origin);
		const float t = numerator / denominator;
		outIntersection = m_origin + (m_direction * t);
		return t >= 0.0f;
	}
	return false;
}
}
