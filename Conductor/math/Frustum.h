#pragma once

#include <math/Matrix4x4.h>
#include <math/Ray3.h>

namespace Math
{
/**
 * Represents a frustum in the world. A frustum looks like a rectangular pyramid with the top chopped off.
 * Frustums are typically used to represent the space a camera can see.
 */
class Frustum
{
public:
	Frustum() = default;

	Frustum(const Math::Matrix4x4& frustumToWorldMatrix,
		float nearDistance,
		float farDistance,
		float verticalFOVRadians,
		float aspectRatio)
		: m_frustumToWorldMatrix(frustumToWorldMatrix)
		, m_nearDistance(nearDistance)
		, m_farDistance(farDistance)
		, m_verticalFieldOfViewRadians(verticalFOVRadians)
		, m_aspectRatio(aspectRatio)
	{}

	// Project through a point on the near plane into the scene. Inputs must be in the range [0, 1].
	Ray3 ProjectThroughNearPlane(const float screenX, const float screenY) const;

	Math::Matrix4x4 m_frustumToWorldMatrix{};
	float m_nearDistance{ 0.0f };
	float m_farDistance{ 0.0f };
	float m_verticalFieldOfViewRadians{ 0.0f };
	float m_aspectRatio{ 0.0f };
};
}

// Inline implementations.
namespace Math
{
inline Ray3 Frustum::ProjectThroughNearPlane(const float screenX, const float screenY) const
{
	const float tanHalfFOV = tanf(m_verticalFieldOfViewRadians * 0.5f);
	const float halfNearPlaneHeight = m_nearDistance * tanHalfFOV;
	const float halfNearPlaneWidth = m_aspectRatio * halfNearPlaneHeight;

	const float xOnNearPlane = (screenX * halfNearPlaneWidth * 2.0f) - halfNearPlaneWidth;
	const float yOnNearPlane = ((1.0f - screenY) * halfNearPlaneHeight * 2.0f) - halfNearPlaneHeight;

	const Math::Matrix4x4 nearToFrustumMatrix = Math::Matrix4x4::MakeTranslation(
		xOnNearPlane, yOnNearPlane, m_nearDistance);

	const Math::Matrix4x4 rayMatrix = m_frustumToWorldMatrix * nearToFrustumMatrix;
	const Math::Vector3 rayOrigin = rayMatrix.GetTranslation();
	const Math::Vector3 rayOffset = rayOrigin - m_frustumToWorldMatrix.GetTranslation();
	const Math::Vector3 rayDirection = rayOffset / rayOffset.Length();

	return Ray3(m_frustumToWorldMatrix.GetTranslation(), rayDirection);
}
}
