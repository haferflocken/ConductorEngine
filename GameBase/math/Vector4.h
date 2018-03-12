#pragma once

namespace Math
{
class Vector4
{
public:
	// Zero vector constructor.
	Vector4();

	// Element-specifying constructor.
	Vector4(float _x, float _y, float _z, float _w);

	Vector4(const Vector4&) = default;
	Vector4& operator=(const Vector4&) = default;

	float& operator[](const size_t i) { return elements[i]; }
	const float& operator[](const size_t i) const { return elements[i]; }

	union
	{
		float elements[4];
		struct
		{
			float x;
			float y;
			float z;
			float w;
		};
	};
};

inline Vector4::Vector4()
	: x(0.0f)
	, y(0.0f)
	, z(0.0f)
	, w(0.0f)
{}

inline Vector4::Vector4(float _x, float _y, float _z, float _w)
	: x(_x)
	, y(_y)
	, z(_z)
	, w(_w)
{}
}
