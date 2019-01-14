#pragma once

#include <cfloat>

namespace Math
{
class Vector4
{
public:
	// Max value constructor.
	Vector4();

	// Element-specifying constructor.
	Vector4(float _x, float _y, float _z, float _w);

	Vector4(const Vector4&) = default;
	Vector4& operator=(const Vector4&) = default;

	float LengthSquared() const;
	float Length() const;

	void operator+=(const Vector4& rhs);
	void operator-=(const Vector4& rhs);
	void operator*=(float rhs);
	void operator/=(float rhs);

	Vector4 operator+(const Vector4& rhs) const;
	Vector4 operator-(const Vector4& rhs) const;
	Vector4 operator*(float rhs) const;
	Vector4 operator/(float rhs) const;

	float Dot(const Vector4& rhs) const;

	float x;
	float y;
	float z;
	float w;
};

inline Vector4::Vector4()
	: x(FLT_MAX)
	, y(FLT_MAX)
	, z(FLT_MAX)
	, w(FLT_MAX)
{}

inline Vector4::Vector4(float _x, float _y, float _z, float _w)
	: x(_x)
	, y(_y)
	, z(_z)
	, w(_w)
{}

inline float Vector4::LengthSquared() const
{
	return (x * x) + (y * y) + (z * z) + (w * w);
}

inline float Vector4::Length() const
{
	return sqrtf(LengthSquared());
}

inline void Vector4::operator+=(const Vector4& rhs)
{
	x += rhs.x;
	y += rhs.y;
	z += rhs.z;
	w += rhs.w;
}

inline void Vector4::operator-=(const Vector4& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
	z -= rhs.z;
	w -= rhs.w;
}

inline void Vector4::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;
	z *= rhs;
	w *= rhs;
}

inline void Vector4::operator/=(float rhs)
{
	x /= rhs;
	y /= rhs;
	z /= rhs;
	w /= rhs;
}

inline Vector4 Vector4::operator+(const Vector4& rhs) const
{
	return Vector4(x + rhs.x, y + rhs.y, z + rhs.z, w + rhs.w);
}

inline Vector4 Vector4::operator-(const Vector4& rhs) const
{
	return Vector4(x - rhs.x, y - rhs.y, z - rhs.z, w - rhs.w);
}

inline Vector4 Vector4::operator*(float rhs) const
{
	return Vector4(x * rhs, y * rhs, z * rhs, w * rhs);
}

inline Vector4 Vector4::operator/(float rhs) const
{
	return Vector4(x / rhs, y / rhs, z / rhs, w / rhs);
}
inline float Vector4::Dot(const Vector4& rhs) const
{
	const float result = (x * rhs.x) + (y * rhs.y) + (z * rhs.z) + (w * rhs.w);
	return result;
}
}
