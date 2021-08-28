#pragma once

#include "../../l4d2Simple2/vector.h"

#include <Windows.h>
#include <limits>

class matrix3x4_t
{
public:
	matrix3x4_t() {}

	inline matrix3x4_t(float m00, float m01, float m02, float m03, float m10,
		float m11, float m12, float m13, float m20, float m21, float m22, float m23)
	{
		m_flMatVal[0][0] = m00; m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
		m_flMatVal[1][0] = m10; m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
		m_flMatVal[2][0] = m20; m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
	}

	inline void Init(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector& vecOrigin)
	{
		m_flMatVal[0][0] = xAxis.x; m_flMatVal[0][1] = yAxis.x; m_flMatVal[0][2] = zAxis.x; m_flMatVal[0][3] = vecOrigin.x;
		m_flMatVal[1][0] = xAxis.y; m_flMatVal[1][1] = yAxis.y; m_flMatVal[1][2] = zAxis.y; m_flMatVal[1][3] = vecOrigin.y;
		m_flMatVal[2][0] = xAxis.z; m_flMatVal[2][1] = yAxis.z; m_flMatVal[2][2] = zAxis.z; m_flMatVal[2][3] = vecOrigin.z;
	}

	inline matrix3x4_t(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis, const Vector& vecOrigin)
	{
		Init(xAxis, yAxis, zAxis, vecOrigin);
	}

	inline void SetOrigin(Vector const& p)
	{
		m_flMatVal[0][3] = p.x;
		m_flMatVal[1][3] = p.y;
		m_flMatVal[2][3] = p.z;
	}

	inline void Invalidate(void)
	{
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 4; j++)
			{
				m_flMatVal[i][j] = std::numeric_limits<float>::infinity();;
			}
		}
	}

	inline Vector GetXAxis() const
	{
		return at(0);
	}

	inline Vector GetYAxis() const
	{
		return at(1);
	}

	inline Vector GetZAxis() const
	{
		return at(2);
	}

	inline Vector GetOrigin() const
	{
		return at(3);
	}

	inline Vector at(int i) const
	{
		return Vector{ m_flMatVal[0][i], m_flMatVal[1][i], m_flMatVal[2][i] };
	}

	inline float* operator[](int i)
	{
		return m_flMatVal[i];
	}

	inline const float* operator[](int i) const
	{
		return m_flMatVal[i];
	}

	inline float* Base()
	{
		return &m_flMatVal[0][0];
	}

	inline const float* Base() const
	{
		return &m_flMatVal[0][0];
	}

	float m_flMatVal[3][4];
};

class VMatrix
{
public:
	VMatrix();

	inline VMatrix(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);

	VMatrix(const Vector& forward, const Vector& left, const Vector& up);
	VMatrix(const matrix3x4_t& matrix3x4);

	void Init(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33);

	void Init(const matrix3x4_t& matrix3x4);

	inline float* operator[](int i)
	{
		return m[i];
	}

	inline const float* operator[](int i) const
	{
		return m[i];
	}

	inline float* Base()
	{
		return &m[0][0];
	}

	inline const float* Base() const
	{
		return &m[0][0];
	}

	void SetLeft(const Vector& vLeft);
	void SetUp(const Vector& vUp);
	void SetForward(const Vector& vForward);

	void GetBasisVectors(Vector& vForward, Vector& vLeft, Vector& vUp) const;
	void SetBasisVectors(const Vector& vForward, const Vector& vLeft, const Vector& vUp);

	Vector& GetTranslation(Vector& vTrans) const;
	void SetTranslation(const Vector& vTrans);

	void PreTranslate(const Vector& vTrans);
	void PostTranslate(const Vector& vTrans);

	matrix3x4_t& As3x4();
	const matrix3x4_t& As3x4() const;

	void CopyFrom3x4(const matrix3x4_t& m3x4);
	void Set3x4(matrix3x4_t& matrix3x4) const;

	bool operator==(const VMatrix& src) const;

	bool operator!=(const VMatrix& src) const
	{
		return !(*this == src);
	}

	Vector GetLeft() const;
	Vector GetUp() const;
	Vector GetForward() const;
	Vector GetTranslation() const;

public:
	void V3Mul(const Vector& vIn, Vector& vOut) const;

	Vector ApplyRotation(const Vector& vVec) const;

	Vector operator*(const Vector& vVec) const;

	Vector VMul3x3(const Vector& vVec) const;
	Vector VMul3x3Transpose(const Vector& vVec) const;
	Vector VMul4x3(const Vector& vVec) const;
	Vector VMul4x3Transpose(const Vector& vVec) const;

public:

	VMatrix& operator=(const VMatrix& mOther);
	const VMatrix& operator+=(const VMatrix& other);
	VMatrix operator+(const VMatrix& other) const;
	VMatrix operator-(const VMatrix& other) const;
	VMatrix operator-() const;
	VMatrix operator~() const;

public:
	void Identity();
	bool IsIdentity() const;

public:
	float  m[4][4];
};

inline void MatrixGetColumn(const matrix3x4_t& src, int nCol, Vector& pColumn)
{
	pColumn.x = src[0][nCol];
	pColumn.y = src[1][nCol];
	pColumn.z = src[2][nCol];
}

inline void MatrixPosition(const matrix3x4_t& matrix, Vector& position)
{
	MatrixGetColumn(matrix, 3, position);
}

inline VMatrix::VMatrix()
{

}

inline VMatrix::VMatrix(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	Init(m00, m01, m02, m03, m10, m11, m12, m13, m20, m21, m22, m23, m30, m31, m32, m33);
}

inline VMatrix::VMatrix(const matrix3x4_t& matrix3x4)
{
	Init(matrix3x4);
}

inline VMatrix::VMatrix(const Vector& xAxis, const Vector& yAxis, const Vector& zAxis)
{
	Init(xAxis.x, yAxis.x, zAxis.x, 0.0f, xAxis.y, yAxis.y, zAxis.y, 0.0f,
		xAxis.z, yAxis.z, zAxis.z, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f);
}

inline void VMatrix::Init(float m00, float m01, float m02, float m03, float m10, float m11, float m12, float m13,
	float m20, float m21, float m22, float m23, float m30, float m31, float m32, float m33)
{
	m[0][0] = m00;
	m[0][1] = m01;
	m[0][2] = m02;
	m[0][3] = m03;

	m[1][0] = m10;
	m[1][1] = m11;
	m[1][2] = m12;
	m[1][3] = m13;

	m[2][0] = m20;
	m[2][1] = m21;
	m[2][2] = m22;
	m[2][3] = m23;

	m[3][0] = m30;
	m[3][1] = m31;
	m[3][2] = m32;
	m[3][3] = m33;
}

inline void VMatrix::Init(const matrix3x4_t& matrix3x4)
{
	memcpy(m, matrix3x4.Base(), sizeof(matrix3x4_t));

	m[3][0] = 0.0f;
	m[3][1] = 0.0f;
	m[3][2] = 0.0f;
	m[3][3] = 1.0f;
}

inline void Vector3DMultiplyPosition(const VMatrix& src1, const Vector& src2, Vector& dst)
{
	dst[0] = src1[0][0] * src2.x + src1[0][1] * src2.y + src1[0][2] * src2.z + src1[0][3];
	dst[1] = src1[1][0] * src2.x + src1[1][1] * src2.y + src1[1][2] * src2.z + src1[1][3];
	dst[2] = src1[2][0] * src2.x + src1[2][1] * src2.y + src1[2][2] * src2.z + src1[2][3];
}

inline Vector VMatrix::GetForward() const
{
	return Vector(m[0][0], m[1][0], m[2][0]);
}

inline Vector VMatrix::GetLeft() const
{
	return Vector(m[0][1], m[1][1], m[2][1]);
}

inline Vector VMatrix::GetUp() const
{
	return Vector(m[0][2], m[1][2], m[2][2]);
}

inline void VMatrix::SetForward(const Vector& vForward)
{
	m[0][0] = vForward.x;
	m[1][0] = vForward.y;
	m[2][0] = vForward.z;
}

inline void VMatrix::SetLeft(const Vector& vLeft)
{
	m[0][1] = vLeft.x;
	m[1][1] = vLeft.y;
	m[2][1] = vLeft.z;
}

inline void VMatrix::SetUp(const Vector& vUp)
{
	m[0][2] = vUp.x;
	m[1][2] = vUp.y;
	m[2][2] = vUp.z;
}

inline void VMatrix::GetBasisVectors(Vector& vForward, Vector& vLeft, Vector& vUp) const
{
	vForward.Init(m[0][0], m[1][0], m[2][0]);
	vLeft.Init(m[0][1], m[1][1], m[2][1]);
	vUp.Init(m[0][2], m[1][2], m[2][2]);
}

inline void VMatrix::SetBasisVectors(const Vector& vForward, const Vector& vLeft, const Vector& vUp)
{
	SetForward(vForward);
	SetLeft(vLeft);
	SetUp(vUp);
}

inline Vector VMatrix::GetTranslation() const
{
	return Vector(m[0][3], m[1][3], m[2][3]);
}

inline Vector& VMatrix::GetTranslation(Vector& vTrans) const
{
	vTrans.x = m[0][3];
	vTrans.y = m[1][3];
	vTrans.z = m[2][3];
	return vTrans;
}

inline void VMatrix::SetTranslation(const Vector& vTrans)
{
	m[0][3] = vTrans.x;
	m[1][3] = vTrans.y;
	m[2][3] = vTrans.z;
}

inline void VMatrix::PreTranslate(const Vector& vTrans)
{
	Vector tmp;
	Vector3DMultiplyPosition(*this, vTrans, tmp);
	m[0][3] = tmp.x;
	m[1][3] = tmp.y;
	m[2][3] = tmp.z;
}

inline void VMatrix::PostTranslate(const Vector& vTrans)
{
	m[0][3] += vTrans.x;
	m[1][3] += vTrans.y;
	m[2][3] += vTrans.z;
}

inline const matrix3x4_t& VMatrix::As3x4() const
{
	return *((const matrix3x4_t*)this);
}

inline matrix3x4_t& VMatrix::As3x4()
{
	return *((matrix3x4_t*)this);
}

inline void VMatrix::CopyFrom3x4(const matrix3x4_t& m3x4)
{
	memcpy(m, m3x4.Base(), sizeof(matrix3x4_t));
	m[3][0] = m[3][1] = m[3][2] = 0;
	m[3][3] = 1;
}

inline void VMatrix::Set3x4(matrix3x4_t& matrix3x4) const
{
	memcpy(matrix3x4.Base(), m, sizeof(matrix3x4_t));
}

inline const VMatrix& VMatrix::operator+=(const VMatrix &other)
{
	for (int i = 0; i < 4; i++) 
	{
		for (int j = 0; j < 4; j++) 
		{
			m[i][j] += other.m[i][j];
		}
	}

	return *this;
}

inline VMatrix VMatrix::operator+(const VMatrix& other) const
{
	VMatrix ret;

	for (int i = 0; i < 16; i++)
	{
		((float*)ret.m)[i] = ((float*)m)[i] + ((float*)other.m)[i];
	}

	return ret;
}

inline VMatrix VMatrix::operator-(const VMatrix& other) const
{
	VMatrix ret;

	for (int i = 0; i < 4; i++)
	{
		for (int j = 0; j < 4; j++)
		{
			ret.m[i][j] = m[i][j] - other.m[i][j];
		}
	}

	return ret;
}

inline VMatrix VMatrix::operator-() const
{
	VMatrix ret;

	for (int i = 0; i < 16; i++)
	{
		((float*)ret.m)[i] = -((float*)m)[i];
	}

	return ret;
}

inline Vector VMatrix::operator*(const Vector& vVec) const
{
	Vector vRet;
	vRet.x = m[0][0] * vVec.x + m[0][1] * vVec.y + m[0][2] * vVec.z + m[0][3];
	vRet.y = m[1][0] * vVec.x + m[1][1] * vVec.y + m[1][2] * vVec.z + m[1][3];
	vRet.z = m[2][0] * vVec.x + m[2][1] * vVec.y + m[2][2] * vVec.z + m[2][3];

	return vRet;
}

inline Vector VMatrix::VMul4x3(const Vector& vVec) const
{
	Vector vResult;
	Vector3DMultiplyPosition(*this, vVec, vResult);
	return vResult;
}

inline Vector VMatrix::VMul4x3Transpose(const Vector& vVec) const
{
	Vector tmp = vVec;
	tmp.x -= m[0][3];
	tmp.y -= m[1][3];
	tmp.z -= m[2][3];

	return Vector(m[0][0] * tmp.x + m[1][0] * tmp.y + m[2][0] * tmp.z,
		m[0][1] * tmp.x + m[1][1] * tmp.y + m[2][1] * tmp.z, m[0][2] * tmp.x + m[1][2] * tmp.y + m[2][2] * tmp.z);
}

inline Vector VMatrix::VMul3x3(const Vector& vVec) const
{
	return Vector(m[0][0] * vVec.x + m[0][1] * vVec.y + m[0][2] * vVec.z,
		m[1][0] * vVec.x + m[1][1] * vVec.y + m[1][2] * vVec.z, m[2][0] * vVec.x + m[2][1] * vVec.y + m[2][2] * vVec.z);
}

inline Vector VMatrix::VMul3x3Transpose(const Vector& vVec) const
{
	return Vector(m[0][0] * vVec.x + m[1][0] * vVec.y + m[2][0] * vVec.z,
		m[0][1] * vVec.x + m[1][1] * vVec.y + m[2][1] * vVec.z, m[0][2] * vVec.x + m[1][2] * vVec.y + m[2][2] * vVec.z);
}

inline void VMatrix::V3Mul(const Vector& vIn, Vector& vOut) const
{
	float rw;

	rw = 1.0f / (m[3][0] * vIn.x + m[3][1] * vIn.y + m[3][2] * vIn.z + m[3][3]);
	vOut.x = (m[0][0] * vIn.x + m[0][1] * vIn.y + m[0][2] * vIn.z + m[0][3]) * rw;
	vOut.y = (m[1][0] * vIn.x + m[1][1] * vIn.y + m[1][2] * vIn.z + m[1][3]) * rw;
	vOut.z = (m[2][0] * vIn.x + m[2][1] * vIn.y + m[2][2] * vIn.z + m[2][3]) * rw;
}

inline void VMatrix::Identity()
{
	m[0][0] = 1.0f; m[0][1] = 0.0f; m[0][2] = 0.0f; m[0][3] = 0.0f;
	m[1][0] = 0.0f; m[1][1] = 1.0f; m[1][2] = 0.0f; m[1][3] = 0.0f;
	m[2][0] = 0.0f; m[2][1] = 0.0f; m[2][2] = 1.0f; m[2][3] = 0.0f;
	m[3][0] = 0.0f; m[3][1] = 0.0f; m[3][2] = 0.0f; m[3][3] = 1.0f;
}

inline bool VMatrix::IsIdentity() const
{
	return m[0][0] == 1.0f && m[0][1] == 0.0f && m[0][2] == 0.0f && m[0][3] == 0.0f &&
		m[1][0] == 0.0f && m[1][1] == 1.0f && m[1][2] == 0.0f && m[1][3] == 0.0f &&
		m[2][0] == 0.0f && m[2][1] == 0.0f && m[2][2] == 1.0f && m[2][3] == 0.0f &&
		m[3][0] == 0.0f && m[3][1] == 0.0f && m[3][2] == 0.0f && m[3][3] == 1.0f;
}

inline Vector VMatrix::ApplyRotation(const Vector& vVec) const
{
	return VMul3x3(vVec);
}