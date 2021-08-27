#pragma once

#include <cmath>

typedef float vec_t;

#define FLOAT32_NAN_BITS (unsigned long)0x7FC00000
#define FLOAT32_NAN BitsToFloat(FLOAT32_NAN_BITS)

#define VEC_T_NAN FLOAT32_NAN

#define INVALID_VECTOR Vector(VEC_T_NAN, VEC_T_NAN, VEC_T_NAN)
#define INVALID_QANGLE QAngle(VEC_T_NAN, VEC_T_NAN, VEC_T_NAN)

#define NULL_VECTOR INVALID_VECTOR
#define NULL_QANGLE INVALID_QANGLE

#define ALLOW_CAST_POINTER
// #define PRIVATE_VECTOR_TYPE

class QAngle;

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

	vec_t Dot(const Vector& v) const;
	Vector Cross(const Vector& v) const;

	Vector Normalize() const;

	vec_t Length() const;
	vec_t Length2D() const;
	vec_t LengthSqr() const;
	vec_t Length2DSqr() const;

	Vector Scale(vec_t f) const;

	QAngle toAngles() const;

	vec_t DistTo(const Vector& vOther) const;
	vec_t DistToSqr(const Vector& vOther) const;

#ifdef ALLOW_CAST_POINTER
	operator vec_t* ();
#endif

public:
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

class QAngle
{
public:
	QAngle(vec_t x = 0.0f, vec_t y = 0.0f, vec_t z = 0.0f);
	QAngle(const Vector& vector);

	void Invalidate();
	bool IsValid() const;
	bool IsZero(float tolerance = 1e-6f) const;
	void Init(vec_t x = 0.0f, vec_t y = 0.0f, vec_t z = 0.0f);

	Vector Forward() const;
	Vector Right() const;
	Vector Up() const;

	QAngle Normalize() const;
	QAngle Clamp() const;

public:
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

inline unsigned long& FloatBits(float& f)
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