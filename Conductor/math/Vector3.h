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

	float& operator[](const size_t i) { return elements[i]; }
	const float& operator[](const size_t i) const { return elements[i]; }

	union
	{
		float elements[3];
		struct
		{
			float x;
			float y;
			float z;
		};
	};
};

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
}
