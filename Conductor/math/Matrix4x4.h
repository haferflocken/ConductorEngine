#pragma once

#include <math/Vector3.h>

namespace Math
{
class Vector4;

class Matrix4x4
{
public:
	// Identity matrix constructor.
	Matrix4x4();

	Matrix4x4(const Matrix4x4&) = default;
	Matrix4x4& operator=(const Matrix4x4&) = default;

	const float* GetData() const { return m_matrix; }

	const Vector3& GetTranslation() const { return reinterpret_cast<const Vector3&>(m_matrix[12]); }
	void SetTranslation(const Vector3& v) { reinterpret_cast<Vector3&>(m_matrix[12]) = v; }

	Vector4& operator[](const size_t i) { return reinterpret_cast<Vector4&>(m_matrix[i * 4]); }
	const Vector4& operator[](const size_t i) const { return reinterpret_cast<const Vector4&>(m_matrix[i * 4]); }

private:
	float m_matrix[16];
};

inline Matrix4x4::Matrix4x4()
{
	for (auto& v : m_matrix)
	{
		v = 0.0f;
	}
	m_matrix[0] = 1.0f;
	m_matrix[5] = 1.0f;
	m_matrix[10] = 1.0f;
	m_matrix[15] = 1.0f;
}
}
