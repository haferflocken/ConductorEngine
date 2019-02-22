#pragma once

#include <cfloat>
#include <cmath>

namespace Math
{
class Vector2
{
public:
	// Max value constructor.
	Vector2();

	// Element-specifying constructor.
	Vector2(float _x, float _y);

	Vector2(const Vector2&) = default;
	Vector2& operator=(const Vector2&) = default;

	float LengthSquared() const;
	float Length() const;

	void operator+=(const Vector2& rhs);
	void operator-=(const Vector2& rhs);
	void operator*=(float rhs);
	void operator/=(float rhs);

	Vector2 operator+(const Vector2& rhs) const;
	Vector2 operator-(const Vector2& rhs) const;
	Vector2 operator*(float rhs) const;
	Vector2 operator/(float rhs) const;

	float Dot(const Vector2& rhs) const;

	bool operator==(const Vector2& rhs) const;
	bool operator!=(const Vector2& rhs) const;

	float x;
	float y;
};
}

// Inline implementations.
namespace Math
{
inline Vector2::Vector2()
	: x(FLT_MAX)
	, y(FLT_MAX)
{}

inline Vector2::Vector2(float _x, float _y)
	: x(_x)
	, y(_y)
{}

inline float Vector2::LengthSquared() const
{
	return (x * x) + (y * y);
}

inline float Vector2::Length() const
{
	return sqrtf(LengthSquared());
}

inline void Vector2::operator+=(const Vector2& rhs)
{
	x += rhs.x;
	y += rhs.y;
}

inline void Vector2::operator-=(const Vector2& rhs)
{
	x -= rhs.x;
	y -= rhs.y;
}

inline void Vector2::operator*=(float rhs)
{
	x *= rhs;
	y *= rhs;
}

inline void Vector2::operator/=(float rhs)
{
	x /= rhs;
	y /= rhs;
}

inline Vector2 Vector2::operator+(const Vector2& rhs) const
{
	return Vector2(x + rhs.x, y + rhs.y);
}

inline Vector2 Vector2::operator-(const Vector2& rhs) const
{
	return Vector2(x - rhs.x, y - rhs.y);
}

inline Vector2 Vector2::operator*(float rhs) const
{
	return Vector2(x * rhs, y * rhs);
}

inline Vector2 Vector2::operator/(float rhs) const
{
	return Vector2(x / rhs, y / rhs);
}

inline float Vector2::Dot(const Vector2& rhs) const
{
	return (x * rhs.x) + (y * rhs.y);
}

inline bool Vector2::operator==(const Vector2& rhs) const
{
	return (x == rhs.x) && (y == rhs.y);
}

inline bool Vector2::operator!=(const Vector2& rhs) const
{
	return !(*this == rhs);
}
}
