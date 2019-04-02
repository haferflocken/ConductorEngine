#pragma once

#include <math/Vector3.h>
#include <math/Vector4.h>

namespace Math
{
/**
 * A 4x4 column-major matrix.
 */
class Matrix4x4
{
public:
	static Matrix4x4 MakeTranslation(const float x, const float y, const float z);
	static Matrix4x4 MakeScale(const float xScale, const float yScale, const float zScale);
	static Matrix4x4 MakeRotateX(const float radians);
	static Matrix4x4 MakeRotateY(const float radians);
	static Matrix4x4 MakeRotateZ(const float radians);
	static Matrix4x4 MakeRotateXYZ(const float xRadians, const float yRadians, const float zRadians);
	static Matrix4x4 MakeOrientZAlong(const Math::Vector3& up, const Math::Vector3& orientAlong);

	// Identity matrix constructor.
	Matrix4x4();

	Matrix4x4(const Matrix4x4&) = default;
	Matrix4x4& operator=(const Matrix4x4&) = default;

	const float* GetData() const { return m_matrix; }

	const Vector3& GetTranslation() const { return reinterpret_cast<const Vector3&>(m_matrix[12]); }
	void SetTranslation(const float x, const float y, const float z);

	Vector3 GetScale() const { return Vector3(m_matrix[0], m_matrix[5], m_matrix[10]); }
	void SetScale(const float xScale, const float yScale, const float zScale);

	Vector4& GetColumn(const size_t i) { return reinterpret_cast<Vector4&>(m_matrix[i * 4]); }
	const Vector4& GetColumn(const size_t i) const { return reinterpret_cast<const Vector4&>(m_matrix[i * 4]); }

	Vector4 GetRow(const size_t i) const { return Vector4(m_matrix[i], m_matrix[i + 4], m_matrix[i + 8], m_matrix[i + 12]); }

	Matrix4x4 Transpose() const;

	float CalcDeterminant() const;
	Matrix4x4 CalcAdjugate() const;
	Matrix4x4 CalcInverse() const;

	Vector3 operator*(const Vector3& rhs) const;

	Matrix4x4 operator*(const Matrix4x4& rhs) const;
	Matrix4x4& operator*=(const Matrix4x4& rhs);

	Matrix4x4 operator*(const float rhs) const;
	Matrix4x4& operator*=(const float rhs);

private:
	float m_matrix[16];
};
}

// Inline implementations.
namespace Math
{
inline Matrix4x4 Matrix4x4::MakeTranslation(const float x, const float y, const float z)
{
	Matrix4x4 result;
	result.SetTranslation(x, y, z);
	return result;
}

inline Matrix4x4 Matrix4x4::MakeScale(const float xScale, const float yScale, const float zScale)
{
	Matrix4x4 result;
	result.SetScale(xScale, yScale, zScale);
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

inline Matrix4x4 Matrix4x4::MakeRotateXYZ(const float xRadians, const float yRadians, const float zRadians)
{
	return MakeRotateZ(zRadians) * MakeRotateY(yRadians) * MakeRotateX(xRadians);
}

inline Matrix4x4 Matrix4x4::MakeOrientZAlong(const Math::Vector3& up, const Math::Vector3& orientAlong)
{
	Matrix4x4 result;

	const Vector3 xAxis = up.Cross(orientAlong);
	const Vector3 yAxis = orientAlong.Cross(xAxis);
	
	Vector4& xColumn = result.GetColumn(0);
	xColumn.x = xAxis.x;
	xColumn.y = xAxis.y;
	xColumn.z = xAxis.z;

	Vector4& yColumn = result.GetColumn(1);
	yColumn.x = yAxis.x;
	yColumn.y = yAxis.y;
	yColumn.z = yAxis.z;

	Vector4& zColumn = result.GetColumn(2);
	zColumn.x = orientAlong.x;
	zColumn.y = orientAlong.y;
	zColumn.z = orientAlong.z;

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

inline void Matrix4x4::SetTranslation(const float x, const float y, const float z)
{
	m_matrix[12] = x;
	m_matrix[13] = y;
	m_matrix[14] = z;
}

inline void Matrix4x4::SetScale(const float xScale, const float yScale, const float zScale)
{
	m_matrix[0] = xScale;
	m_matrix[5] = yScale;
	m_matrix[10] = zScale;
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

inline float Matrix4x4::CalcDeterminant() const
{
	const float& a11 = m_matrix[0];
	const float& a21 = m_matrix[1];
	const float& a31 = m_matrix[2];
	const float& a41 = m_matrix[3];

	const float& a12 = m_matrix[4];
	const float& a22 = m_matrix[5];
	const float& a32 = m_matrix[6];
	const float& a42 = m_matrix[7];

	const float& a13 = m_matrix[8];
	const float& a23 = m_matrix[9];
	const float& a33 = m_matrix[10];
	const float& a43 = m_matrix[11];

	const float& a14 = m_matrix[12];
	const float& a24 = m_matrix[13];
	const float& a34 = m_matrix[14];
	const float& a44 = m_matrix[15];

	const float r0 = (a11 * a22 * a33 * a44) + (a11 * a23 * a34 * a42) + (a11 * a24 * a32 * a43);
	const float r1 = -(a11 * a24 * a33 * a42) - (a11 * a23 * a32 * a44) - (a11 * a22 * a34 * a43);
	const float r2 = -(a12 * a21 * a33 * a44) - (a13 * a21 * a34 * a42) - (a14 * a21 * a32 * a43);
	const float r3 = (a14 * a21 * a33 * a42) + (a13 * a21 * a32 * a44) + (a12 * a21 * a34 * a43);

	const float r4 = (a12 * a23 * a31 * a44) + (a13 * a24 * a31 * a42) + (a14 * a22 * a31 * a43);
	const float r5 = -(a14 * a23 * a31 * a42) - (a13 * a22 * a31 * a44) - (a12 * a24 * a31 * a43);
	const float r6 = -(a12 * a23 * a34 * a41) - (a13 * a24 * a32 * a41) - (a14 * a22 * a33 * a41);
	const float r7 = (a14 * a23 * a32 * a41) + (a13 * a22 * a34 * a41) + (a12 * a24 * a33 * a41);

	return r0 + r1 + r2 + r3 + r4 + r5 + r6 + r7;

}

inline Matrix4x4 Matrix4x4::CalcAdjugate() const
{
	const float& a11 = m_matrix[0];
	const float& a21 = m_matrix[1];
	const float& a31 = m_matrix[2];
	const float& a41 = m_matrix[3];

	const float& a12 = m_matrix[4];
	const float& a22 = m_matrix[5];
	const float& a32 = m_matrix[6];
	const float& a42 = m_matrix[7];

	const float& a13 = m_matrix[8];
	const float& a23 = m_matrix[9];
	const float& a33 = m_matrix[10];
	const float& a43 = m_matrix[11];

	const float& a14 = m_matrix[12];
	const float& a24 = m_matrix[13];
	const float& a34 = m_matrix[14];
	const float& a44 = m_matrix[15];

	Matrix4x4 result;
	Vector4& column0 = result.GetColumn(0);
	Vector4& column1 = result.GetColumn(1);
	Vector4& column2 = result.GetColumn(2);
	Vector4& column3 = result.GetColumn(3);

	column0.x = (a22 * a33 * a44) + (a23 * a34 * a42) + (a24 * a32 * a43)
		- (a24 * a33 * a42) - (a23 * a32 * a44) - (a22 * a34 * a43);
	column1.x = -(a12 * a33 * a44) - (a13 * a34 * a42) - (a14 * a32 * a43)
		+ (a14 * a33 * a42) + (a13 * a32 * a44) + (a12 * a34 * a43);
	column2.x = (a12 * a23 * a44) + (a13 * a24 * a42) + (a14 * a22 * a43)
		- (a14 * a23 * a42) - (a13 * a22 * a44) - (a12 * a24 * a43);
	column3.x = -(a12 * a23 * a34) - (a13 * a24 * a32) - (a14 * a22 * a33)
		+ (a14 * a23 * a32) + (a13 * a22 * a34) + (a12 * a24 * a33);

	column0.y = -(a21 * a33 * a44) - (a23 * a34 * a41) - (a24 * a31 * a43)
		+ (a24 * a33 * a41) + (a23 * a31 * a44) + (a21 * a34 * a43);
	column1.y = (a11 * a33 * a44) + (a13 * a34 * a41) + (a14 * a31 * a43)
		- (a14 * a33 * a41) - (a13 * a31 * a44) - (a11 * a34 * a43);
	column2.y = -(a11 * a23 * a44) - (a13 * a24 * a41) - (a14 * a21 * a43)
		+ (a14 * a23 * a41) + (a13 * a21 * a44) + (a11 * a24 * a43);
	column3.y = (a11 * a23 * a34) + (a13 * a24 * a31) + (a14 * a21 * a33)
		- (a14 * a23 * a31) - (a13 * a21 * a34) - (a11 * a24 * a33);

	column0.z = (a21 * a32 * a44) + (a22 * a34 * a41) + (a24 * a31 * a42)
		- (a24 * a32 * a41) - (a22 * a31 * a44) - (a21 * a34 * a42);
	column1.z = -(a11 * a32 * a44) - (a12 * a34 * a41) - (a14 * a31 * a42)
		+ (a14 * a32 * a41) + (a12 * a31 * a44) + (a11 * a34 * a42);
	column2.z = (a11 * a22 * a44) + (a12 * a24 * a41) + (a14 * a21 * a42)
		- (a14 * a22 * a41) - (a12 * a21 * a44) - (a11 * a24 * a42);
	column3.z = -(a11 * a22 * a34) - (a12 * a24 * a31) - (a14 * a21 * a32)
		+ (a14 * a22 * a31) + (a12 * a21 * a34) + (a11 * a24 * a32);

	column0.w = -(a21 * a32 * a43) - (a22 * a33 * a41) - (a23 * a31 * a42)
		+ (a23 * a32 * a41) + (a22 * a31 * a43) + (a21 * a33 * a42);
	column1.w = (a11 * a32 * a43) + (a12 * a33 * a41) + (a13 * a31 * a42)
		-  (a13 * a32 * a41) - (a12 * a31 * a43) - (a11 * a33 * a42);
	column2.w = -(a11 * a22 * a43) - (a12 * a23 * a41) - (a13 * a21 * a42)
		+ (a13 * a22 * a41) + (a12 * a21 * a43) + (a11 * a23 * a42);
	column3.w = (a11 * a22 * a33) + (a12 * a23 * a31) + (a13 * a21 * a32)
		- (a13 * a22 * a31) - (a12 * a21 * a33) - (a11 * a23 * a32);

	return result;
}

inline Matrix4x4 Matrix4x4::CalcInverse() const
{
	const float determinant = CalcDeterminant();
	const Matrix4x4 adjugateMatrix = CalcAdjugate();
	return adjugateMatrix * (1.0f / determinant);
}

inline Vector3 Matrix4x4::operator*(const Vector3& rhs) const
{
	const float& row0X = m_matrix[0];
	const float& row0Y = m_matrix[4];
	const float& row0Z = m_matrix[8];
	const float& row0W = m_matrix[12];

	const float& row1X = m_matrix[1];
	const float& row1Y = m_matrix[5];
	const float& row1Z = m_matrix[9];
	const float& row1W = m_matrix[13];

	const float& row2X = m_matrix[2];
	const float& row2Y = m_matrix[6];
	const float& row2Z = m_matrix[10];
	const float& row2W = m_matrix[14];

	Vector3 result;
	result.x = (row0X * rhs.x) + (row0Y * rhs.y) + (row0Z * rhs.z) + row0W;
	result.y = (row1X * rhs.x) + (row1Y * rhs.y) + (row1Z * rhs.z) + row1W;
	result.z = (row2X * rhs.x) + (row2Y * rhs.y) + (row2Z * rhs.z) + row2W;
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
	const Matrix4x4 current = *this;
	*this = current * rhs;
	return *this;
}

inline Matrix4x4 Matrix4x4::operator*(const float rhs) const
{
	Matrix4x4 result = *this;
	result *= rhs;
	return result;
}

inline Matrix4x4& Matrix4x4::operator*=(const float rhs)
{
	for (auto& v : m_matrix)
	{
		v *= rhs;
	}
	return *this;
}
}
