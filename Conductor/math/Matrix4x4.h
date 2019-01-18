#pragma once

#include <math/Vector3.h>
#include <math/Vector4.h>

namespace Math
{
class Vector4;

/**
 * A 4x4 column-major matrix.
 */
class Matrix4x4
{
public:
	static Matrix4x4 MakeTranslation(const Vector3& translation);
	static Matrix4x4 MakeScale(const Vector3& scale);
	static Matrix4x4 MakeRotateX(const float radians);
	static Matrix4x4 MakeRotateY(const float radians);
	static Matrix4x4 MakeRotateZ(const float radians);

	// Identity matrix constructor.
	Matrix4x4();

	Matrix4x4(const Matrix4x4&) = default;
	Matrix4x4& operator=(const Matrix4x4&) = default;

	const float* GetData() const { return m_matrix; }

	const Vector3& GetTranslation() const { return reinterpret_cast<const Vector3&>(m_matrix[12]); }
	void SetTranslation(const Vector3& v) { reinterpret_cast<Vector3&>(m_matrix[12]) = v; }

	Vector3 GetScale() const { return Vector3(m_matrix[0], m_matrix[5], m_matrix[10]); }
	void SetScale(const Vector3& v);

	Vector4& GetColumn(const size_t i) { return reinterpret_cast<Vector4&>(m_matrix[i * 4]); }
	const Vector4& GetColumn(const size_t i) const { return reinterpret_cast<const Vector4&>(m_matrix[i * 4]); }

	Vector4 GetRow(const size_t i) const { return Vector4(m_matrix[i], m_matrix[i + 4], m_matrix[i + 8], m_matrix[i + 12]); }

	Matrix4x4 Transpose() const;

	Matrix4x4 operator*(const Matrix4x4& rhs) const;
	Matrix4x4& operator*=(const Matrix4x4& rhs);

private:
	float m_matrix[16];
};

inline Matrix4x4 Matrix4x4::MakeTranslation(const Vector3& translation)
{
	Matrix4x4 result;
	result.SetTranslation(translation);
	return result;
}

inline Matrix4x4 Matrix4x4::MakeScale(const Vector3& scale)
{
	Matrix4x4 result;
	result.SetScale(scale);
	return result;
}

inline Matrix4x4 Matrix4x4::MakeRotateX(const float radians)
{
	Matrix4x4 result;
	result.m_matrix[5] = cosf(radians);
	result.m_matrix[6] = sinf(radians);
	result.m_matrix[9] = -result.m_matrix[6];
	result.m_matrix[10] = result.m_matrix[5];
	return result;
}

inline Matrix4x4 Matrix4x4::MakeRotateY(const float radians)
{
	Matrix4x4 result;
	result.m_matrix[0] = cosf(radians);
	result.m_matrix[2] = -sinf(radians);
	result.m_matrix[8] = -result.m_matrix[2];
	result.m_matrix[10] = result.m_matrix[0];
	return result;
}

inline Matrix4x4 Matrix4x4::MakeRotateZ(const float radians)
{
	Matrix4x4 result;
	result.m_matrix[0] = cosf(radians);
	result.m_matrix[1] = sinf(radians);
	result.m_matrix[4] = -result.m_matrix[1];
	result.m_matrix[5] = result.m_matrix[0];
	return result;
}

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

inline void Matrix4x4::SetScale(const Vector3& v)
{
	m_matrix[0] = v.x;
	m_matrix[5] = v.y;
	m_matrix[10] = v.z;
}

inline Matrix4x4 Matrix4x4::Transpose() const
{
	Matrix4x4 result;
	result.GetColumn(0) = GetRow(0);
	result.GetColumn(1) = GetRow(1);
	result.GetColumn(2) = GetRow(2);
	result.GetColumn(3) = GetRow(3);
	return result;
}

inline Matrix4x4 Matrix4x4::operator*(const Matrix4x4& rhs) const
{
	Matrix4x4 result;
	
	Vector4& result0 = result.GetColumn(0);
	Vector4& result1 = result.GetColumn(1);
	Vector4& result2 = result.GetColumn(2);
	Vector4& result3 = result.GetColumn(3);

	const Vector4 lhs0 = GetRow(0);
	const Vector4 lhs1 = GetRow(1);
	const Vector4 lhs2 = GetRow(2);
	const Vector4 lhs3 = GetRow(3);

	const Vector4& rhs0 = rhs.GetColumn(0);
	const Vector4& rhs1 = rhs.GetColumn(1);
	const Vector4& rhs2 = rhs.GetColumn(2);
	const Vector4& rhs3 = rhs.GetColumn(3);
	
	// First column.
	result0.x = lhs0.Dot(rhs0);
	result0.y = lhs1.Dot(rhs0);
	result0.z = lhs2.Dot(rhs0);
	result0.w = lhs3.Dot(rhs0);

	// Second column.
	result1.x = lhs0.Dot(rhs1);
	result1.y = lhs1.Dot(rhs1);
	result1.z = lhs2.Dot(rhs1);
	result1.w = lhs3.Dot(rhs1);

	// Third column.
	result2.x = lhs0.Dot(rhs2);
	result2.y = lhs1.Dot(rhs2);
	result2.z = lhs2.Dot(rhs2);
	result2.w = lhs3.Dot(rhs2);

	// Fourth column.
	result3.x = lhs0.Dot(rhs3);
	result3.y = lhs1.Dot(rhs3);
	result3.z = lhs2.Dot(rhs3);
	result3.w = lhs3.Dot(rhs3);

	return result;
}

inline Matrix4x4& Matrix4x4::operator*=(const Matrix4x4& rhs)
{
	Matrix4x4 current = *this;
	*this = current * rhs;
	return *this;
}
}
