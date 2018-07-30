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
}
