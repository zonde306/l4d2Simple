#pragma once
#include <cmath>

typedef float vec_t;

#define FLOAT32_NAN_BITS	(unsigned long)0x7FC00000
#define FLOAT32_NAN			BitsToFloat(FLOAT32_NAN_BITS)
#define VEC_T_NAN			FLOAT32_NAN
#define INVALID_VECTOR Vector(VEC_T_NAN, VEC_T_NAN, VEC_T_NAN)
#define INVALID_QANGLE QAngle(VEC_T_NAN, VEC_T_NAN, VEC_T_NAN)

// #define ALLOW_CAST_POINTER
// #define PRIVATE_VECTOR_TYPE

// 欧拉角
class QAngle;

// 向量
class Vector
{
public:
	Vector(vec_t x = 0.0f, vec_t y = 0.0f, vec_t z = 0.0f);
	Vector(const QAngle& angles);
	Vector(const vec_t* v);

	void Invalidate();
	bool IsValid() const;
	bool IsZero(float tolerance = 1e-6f) const;
	void Init(vec_t x = 0.0f, vec_t y = 0.0f, vec_t z = 0.0f);
	void SetZero();

	// 点乘，返回两个向量之间的角度(相似度)
	vec_t Dot(const Vector& v) const;

	// 叉乘，返回同时垂直于两个向量的新向量
	Vector Cross(const Vector& v) const;

	// 归一化，返回一个长度为 1 的向量
	Vector Normalize() const;

	// 获取向量的长度
	vec_t Length() const;
	vec_t Length2D() const;

	// 获取向量的长度的平方
	vec_t LengthSqr() const;
	vec_t Length2DSqr() const;

	// 缩放向量，相当于乘法
	Vector Scale(vec_t f) const;

	// 向量和欧拉角之间的转换
	QAngle toAngles() const;

	vec_t DistTo(const Vector& vOther) const;
	vec_t DistToSqr(const Vector& vOther) const;

#ifdef ALLOW_CAST_POINTER
	// 某些情况下可能需要
	operator vec_t*();
#endif

public:	// 各类操作符
	float operator[](int index) const;
	float& operator[](int index);

	bool operator==(const Vector& v) const;
	bool operator!=(const Vector& v) const;

	Vector operator+(const Vector& v) const;
	Vector operator-(const Vector& v) const;
	Vector operator*(const Vector& v) const;
	Vector operator/(const Vector& v) const;
	Vector operator+(const vec_t& f) const;
	Vector operator-(const vec_t& f) const;
	Vector operator*(const vec_t& f) const;
	Vector operator/(const vec_t& f) const;
	Vector operator-() const;

	Vector& operator+=(const Vector& v);
	Vector& operator-=(const Vector& v);
	Vector& operator*=(const Vector& v);
	Vector& operator/=(const Vector& v);
	Vector& operator+=(const vec_t& f);
	Vector& operator-=(const vec_t& f);
	Vector& operator*=(const vec_t& f);
	Vector& operator/=(const vec_t& f);

#ifdef PRIVATE_VECTOR_TYPE
private:
#else
public:
#endif
	vec_t x, y, z;
};

// 欧拉角
class QAngle
{
public:
	QAngle(vec_t x = 0.0f, vec_t y = 0.0f, vec_t z = 0.0f);
	QAngle(const Vector& vector);

	void Invalidate();
	bool IsValid() const;
	bool IsZero(float tolerance = 1e-6f) const;
	void Init(vec_t x = 0.0f, vec_t y = 0.0f, vec_t z = 0.0f);

	// 向量和欧拉角之间的转换
	Vector Forward() const;
	Vector Right() const;
	Vector Up() const;

	// 规范化，用于角度超过范围
	// 返回一个正确的角度
	QAngle Normalize() const;

	// 修正角度范围，防止被检测到
	QAngle Clamp() const;

public:	// 各类操作符
	float operator[](int index) const;
	float& operator[](int index);

	bool operator==(const QAngle& v) const;
	bool operator!=(const QAngle& v) const;

	QAngle operator+(const QAngle& v) const;
	QAngle operator-(const QAngle& v) const;
	QAngle operator*(const QAngle& v) const;
	QAngle operator/(const QAngle& v) const;
	QAngle operator+(const vec_t& f) const;
	QAngle operator-(const vec_t& f) const;
	QAngle operator*(const vec_t& f) const;
	QAngle operator/(const vec_t& f) const;
	QAngle operator-() const;

	QAngle& operator+=(const QAngle& v);
	QAngle& operator-=(const QAngle& v);
	QAngle& operator*=(const QAngle& v);
	QAngle& operator/=(const QAngle& v);
	QAngle& operator+=(const vec_t& f);
	QAngle& operator-=(const vec_t& f);
	QAngle& operator*=(const vec_t& f);
	QAngle& operator/=(const vec_t& f);

#ifdef PRIVATE_VECTOR_TYPE
private:
#else
public:
#endif
	vec_t x, y, z;
};


class Vector2D
{
public:
	Vector2D(vec_t x = 0.0f, vec_t y = 0.0f);

#ifdef PRIVATE_VECTOR_TYPE
private:
#else
public:
#endif
	vec_t x, y;
};

class Vector4D
{
public:
	Vector4D(vec_t x = 0.0f, vec_t y = 0.0f, vec_t z = 0.0f, vec_t w = 0.0f);

#ifdef PRIVATE_VECTOR_TYPE
private:
#else
public:
#endif
	vec_t x, y, z, w;
};

inline unsigned long & FloatBits(float & f)
{
	return *reinterpret_cast<unsigned long*>(&f);
}

inline unsigned long const& FloatBits(float const& f)
{
	return *reinterpret_cast<unsigned long const*>(&f);
}

inline float BitsToFloat(unsigned long i)
{
	return *reinterpret_cast<float*>(&i);
}

inline float IsFinite(float f)
{
	return ((FloatBits(f) & 0x7F800000) != 0x7F800000);
}

inline unsigned long FloatAbsBits(float f)
{
	return FloatBits(f) & 0x7FFFFFFF;
}

inline float FloatMakeNegative(float f)
{
	return BitsToFloat(FloatBits(f) | 0x80000000);
}

inline float FloatMakePositive(float f)
{
	return abs(f);
}

inline float FloatNegate(float f)
{
	return BitsToFloat(FloatBits(f) ^ 0x80000000);
}
