#pragma once

#include <cfloat>
#include <cmath>

namespace Math
{
class Vector3
{
public:
	// Max value constructor.
	Vector3();

	// Element-specifying constructor.
	constexpr Vector3(float _x, float _y, float _z);

	Vector3(const Vector3&) = default;
	Vector3& operator=(const Vector3&) = default;

	float LengthSquared() const;
	float Length() const;

	void operator+=(const Vector3& rhs);
	void operator-=(const Vector3& rhs);
	void operator*=(float rhs);
	void operator/=(float rhs);

	Vector3 operator+(const Vector3& rhs) const;
	Vector3 operator-(const Vector3& rhs) const;
	Vector3 operator*(float rhs) const;
	Vector3 operator/(float rhs) const;

	float Dot(const Vector3& rhs) const;
	Vector3 Cross(const Vector3& rhs) const;

	bool operator==(const Vector3& rhs) const;
	bool operator!=(const Vector3& rhs) const;

	float x;
	float y;
	float z;
};
}

// Inline implementations.
namespace Math
{
inline Vector3::Vector3()
	: x(FLT_MAX)
	, y(FLT_MAX)
	, z(FLT_MAX)
{}

inline constexpr Vector3::Vector3(float _x, float _y, float _z)
	: x(_x)
	, y(_y)
	, z(_z)
{}

inline float Vector3::LengthSquared() const
{
	return (x * x) + (y * y) + (z * z);
}

inline float Vector3::Length() const
{
	return sqrtf(LengthSquared());
}

inline void Vector3::operator+=(const Vector3& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
}

inline void Vector3::operator-=(const Vector3& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
}

inline void Vector3::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
}

inline void Vector3::operator/=(float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
}

inline Vector3 Vector3::operator+(const Vector3& rhs) const
{
	return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
}

inline Vector3 Vector3::operator-(const Vector3& rhs) const
{
	return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
}

inline Vector3 Vector3::operator*(float rhs) const
{
	return Vector3(x * rhs, y * rhs, z * rhs);
}

inline Vector3 Vector3::operator/(float rhs) const
{
	return Vector3(x / rhs, y / rhs, z / rhs);
}

inline float Vector3::Dot(const Vector3& rhs) const
{
	return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
}

inline Vector3 Vector3::Cross(const Vector3& rhs) const
{
	Vector3 result;
	result.x = (y * rhs.z) - (z * rhs.y);
	result.y = (z * rhs.x) - (x * rhs.z);
	result.z = (x * rhs.y) - (y * rhs.x);
	return result;
}

inline bool Vector3::operator==(const Vector3& rhs) const
{
	return (x == rhs.x) && (y == rhs.y) && (z == rhs.z);
}

inline bool Vector3::operator!=(const Vector3& rhs) const
{
	return !(*this == rhs);
}
}
