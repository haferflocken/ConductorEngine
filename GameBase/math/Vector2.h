#pragma once

namespace Math
{
class Vector2
{
public:
	// Zero vector constructor.
	Vector2();

	// Element-specifying constructor.
	Vector2(float _x, float _y);

	Vector2(const Vector2&) = default;
	Vector2& operator=(const Vector2&) = default;

	float& operator[](const size_t i) { return elements[i]; }
	const float& operator[](const size_t i) const { return elements[i]; }

	union
	{
		float elements[2];
		struct
		{
			float x;
			float y;
		};
	};
};

inline Vector2::Vector2()
	: x(0.0f)
	, y(0.0f)
{}

inline Vector2::Vector2(float _x, float _y)
	: x(_x)
	, y(_y)
{}
}
