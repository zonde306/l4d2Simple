#pragma once
#include "../../l4d2Simple2/vector.h"
#include <Windows.h>

typedef float VMatrix[4][4];

struct matrix3x4_t
{
public:
	matrix3x4_t() {}
	inline matrix3x4_t(VMatrix mat)
	{
		memcpy_s(this, sizeof(matrix3x4_t), mat, sizeof(matrix3x4_t));
	}

	inline void GetMatrix(VMatrix mat)
	{
		memcpy_s(mat, sizeof(matrix3x4_t), this, sizeof(matrix3x4_t));
	}

	matrix3x4_t(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33);

	void Pad();
	void Pad2();

	void Init
	(float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23,
		float m30, float m31, float m32, float m33);


	void Init(void*);

	inline float* operator[](int i)
	{
		return m[i];
	}

	inline const float* operator[](int i) const
	{
		return m[i];
	}


	inline float *Base()
	{
		return &m[0][0];
	}

	inline const float *Base() const
	{
		return &m[0][0];
	}

	inline Vector operator*(const Vector &vVec) const
	{
		return Vector(m[0][0] * vVec.x + m[0][1] * vVec.y + m[0][2] * vVec.z + m[0][3],
			m[1][0] * vVec.x + m[1][1] * vVec.y + m[1][2] * vVec.z + m[1][3],
			m[2][0] * vVec.x + m[2][1] * vVec.y + m[2][2] * vVec.z + m[2][3]);
	}

	matrix3x4_t& operator=(const matrix3x4_t &mOther);

	bool operator==(const matrix3x4_t& src) const
	{
		return !memcmp(src.m, m, sizeof(m));
	}

	void MatrixMul(const matrix3x4_t &vm, matrix3x4_t &out) const;

	matrix3x4_t operator*(const matrix3x4_t &vm) const
	{
		matrix3x4_t ret;
		MatrixMul(vm, ret);
		return ret;
	}

	inline matrix3x4_t operator+(const matrix3x4_t &other) const
	{
		matrix3x4_t ret;
		for (int i = 0; i < 16; i++)
		{
			((float*)ret.m)[i] = ((float*)m)[i] + ((float*)other.m)[i];
		}
		return ret;
	}

public:
	float m[4][4];
};
