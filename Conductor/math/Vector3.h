#pragma once

namespace Math
{
class Vector3
{
public:
	// Zero vector constructor.
	Vector3();

	// Element-specifying constructor.
	Vector3(float _x, float _y, float _z);

	Vector3(const Vector3&) = default;
	Vector3& operator=(const Vector3&) = default;

	float LengthSquared() const;
	float Length() const;

	Vector3 operator+(const Vector3& rhs) const;
	Vector3 operator-(const Vector3& rhs) const;

	void operator+=(const Vector3& rhs);
	void operator-=(const Vector3& rhs);

	Vector3 operator*(const float rhs) const;
	Vector3 operator/(const float rhs) const;

	void operator*=(const float rhs);
	void operator/=(const float rhs);

	float Dot(const Vector3& rhs) const;

	float x;
	float y;
	float z;
};
}

// Inline implementations.
namespace Math
{
inline Vector3::Vector3()
	: x(0.0f)
	, y(0.0f)
	, z(0.0f)
{}

inline Vector3::Vector3(float _x, float _y, float _z)
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

inline Vector3 Vector3::operator+(const Vector3& rhs) const
{
	return Vector3(x + rhs.x, y + rhs.y, z + rhs.z);
}

inline Vector3 Vector3::operator-(const Vector3& rhs) const
{
	return Vector3(x - rhs.x, y - rhs.y, z - rhs.z);
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

inline Vector3 Vector3::operator*(const float rhs) const
{
	return Vector3(x * rhs, y * rhs, z * rhs);
}

inline Vector3 Vector3::operator/(const float rhs) const
{
	return Vector3(x / rhs, y / rhs, z / rhs);
}

inline void Vector3::operator*=(const float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
}

inline void Vector3::operator/=(const float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
}

inline float Vector3::Dot(const Vector3& rhs) const
{
	return (x * rhs.x) + (y * rhs.y) + (z * rhs.z);
}
}
