#include "vector.h"
#include <cmath>
#include <utility>

#ifndef M_PI
#define M_PI	3.14159265358979323846
#define M_PI_F	((float)M_PI)
#endif

#ifndef RAD2DEG
#define RAD2DEG(x)  ((float)(x) * (float)(180.f / M_PI_F))
#define RadiansToDegrees RAD2DEG
#define DEG2RAD(x)  ((float)(x) * (float)(M_PI_F / 180.f))
#define DegreesToRadians DEG2RAD
#endif

#ifndef Assert
#define Assert(_v)			
#endif

#define CHECK_VALID(_v)		Assert((_v).IsValid())

#ifndef FLOAT32_NAN_BITS
#define FLOAT32_NAN_BITS	(unsigned long)0x7FC00000
#define FLOAT32_NAN			BitsToFloat(FLOAT32_NAN_BITS)
#define VEC_T_NAN			FLOAT32_NAN
#endif

// Math routines done in optimized assembly math package routines
void inline SinCos(float radians, float *sine, float *cosine)
{
	*sine = sin(radians);
	*cosine = cos(radians);
}

Vector::Vector(vec_t x, vec_t y, vec_t z) : x(x), y(y), z(z)
{
}

Vector::Vector(const QAngle & angles)
{
	x = angles.x;
	y = angles.y;
	z = angles.z;
}

void Vector::Invalidate()
{
	x = y = z = VEC_T_NAN;
}

bool Vector::IsValid() const
{
	return (IsFinite(x) && IsFinite(y) && IsFinite(z));
}

bool Vector::IsZero(float tolerance) const
{
	return (x > -tolerance && x < tolerance && y > -tolerance && y < tolerance && z > -tolerance && z < tolerance);
}

void Vector::Init(vec_t x, vec_t y, vec_t z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

void Vector::SetZero()
{
	x = y = z = 0.0f;
}

vec_t Vector::Dot(const Vector & v) const
{
	return (x * v.x + y * v.y + z * v.z);
}

Vector Vector::Cross(const Vector & v) const
{
	return Vector(y * v.z - z * v.y, z * v.x - x * v.z, x * v.y - y * v.x);
}

Vector Vector::Normalize() const
{
	vec_t length = sqrt(x * x + y * y + z * z);
	return Vector(x / length, y / length, z / length);
}

vec_t Vector::Length() const
{
	return sqrt(x * x + y * y + z * z);
}

vec_t Vector::Length2D() const
{
	return sqrt(x * x + y * y);
}

vec_t Vector::LengthSqr() const
{
	return (x * x + y * y + z * z);
}

vec_t Vector::Length2DSqr() const
{
	return (x * x + y * y);
}

Vector Vector::Scale(vec_t f) const
{
	return Vector(x * f, y * f, z * f);
}

QAngle Vector::toAngles() const
{
	vec_t tmp, yaw, pitch;
	if (x == 0.0f && y == 0.0f)
	{
		yaw = 0.0f;
		if (z > 0.0f)
			pitch = 270.0f;
		else
			pitch = 90.0f;
	}
	else
	{
		yaw = static_cast<float>(atan2(y, x) * 180.0f / M_PI);
		if (yaw < 0.0f)
			yaw += 360.0f;

		tmp = sqrt(x * x + y * y);
		pitch = static_cast<float>(atan2(-z, tmp) * 180.0f / M_PI);
		if (pitch < 0.0f)
			pitch += 360.0f;
	}

	return QAngle(pitch, yaw, 0.0f);
}

#ifdef ALLOW_CAST_POINTER
Vector::operator vec_t*()
{
	return &x;
}
#endif

float Vector::operator[](int index) const
{
	if (index <= 0)
		return x;
	if (index == 1)
		return y;
	return z;
}

float & Vector::operator[](int index)
{
	if (index <= 0)
		return x;
	if (index == 1)
		return y;
	return z;
}

bool Vector::operator==(const Vector & v) const
{
	return (x == v.x && y == v.y && z == v.z);
}

bool Vector::operator!=(const Vector & v) const
{
	return (x != v.x || y != v.y || z != v.z);
}

Vector Vector::operator+(const Vector & v) const
{
	return Vector(x + v.x, y + v.y, z + v.z);
}

Vector Vector::operator-(const Vector & v) const
{
	return Vector(x - v.x, y - v.y, z - v.z);
}

Vector Vector::operator*(const Vector & v) const
{
	return Vector(x * v.x, y * v.y, z * v.z);
}

Vector Vector::operator/(const Vector & v) const
{
	return Vector(x / v.x, y / v.y, z / v.z);
}

Vector Vector::operator+(const vec_t & f) const
{
	return Vector(x + f, y + f, z + f);
}

Vector Vector::operator-(const vec_t & f) const
{
	return Vector(x - f, y - f, z - f);
}

Vector Vector::operator*(const vec_t & f) const
{
	return Vector(x * f, y * f, z * f);
}

Vector Vector::operator/(const vec_t & f) const
{
	return Vector(x / f, y / f, z / f);
}

Vector Vector::operator-() const
{
	return Vector(-x, -y, -z);
}

Vector & Vector::operator+=(const Vector & v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

Vector & Vector::operator-=(const Vector & v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

Vector & Vector::operator*=(const Vector & v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

Vector & Vector::operator/=(const Vector & v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}

Vector & Vector::operator+=(const vec_t & f)
{
	x += f;
	y += f;
	z += f;
	return *this;
}

Vector & Vector::operator-=(const vec_t & f)
{
	x -= f;
	y -= f;
	z -= f;
	return *this;
}

Vector & Vector::operator*=(const vec_t & f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

Vector & Vector::operator/=(const vec_t & f)
{
	x /= f;
	y /= f;
	z /= f;
	return *this;
}

QAngle::QAngle(vec_t x, vec_t y, vec_t z) : x(x), y(y), z(z)
{
}

QAngle::QAngle(const Vector & vector)
{
	x = vector.x;
	y = vector.y;
	z = vector.z;
}

void QAngle::Invalidate()
{
	x = y = z = VEC_T_NAN;
}

bool QAngle::IsValid() const
{
	return (IsFinite(x) && IsFinite(y) && IsFinite(z));
}

bool QAngle::IsZero(float tolerance) const
{
	return (x > -tolerance && x < tolerance && y > -tolerance && y < tolerance && z > -tolerance && z < tolerance);
}

Vector QAngle::Forward() const
{
	vec_t sp, sy, cp, cy;
	SinCos(DEG2RAD(y), &sy, &cy);
	SinCos(DEG2RAD(x), &sp, &cp);
	return Vector(cp * cy, cp * sy, -sp);
}

Vector QAngle::Right() const
{
	vec_t sr, sp, sy, cr, cp, cy;
	SinCos(DEG2RAD(y), &sy, &cy);
	SinCos(DEG2RAD(x), &sp, &cp);
	SinCos(DEG2RAD(z), &sr, &cr);
	return Vector(-1 * sr * sp * cy + -1 * cr * -sy,
		-1 * sr * sp * sy + -1 * cr * cy,
		-1 * sr * cp);
}

Vector QAngle::Up() const
{
	vec_t sr, sp, sy, cr, cp, cy;
	SinCos(DEG2RAD(y), &sy, &cy);
	SinCos(DEG2RAD(x), &sp, &cp);
	SinCos(DEG2RAD(z), &sr, &cr);
	return Vector(cr * sp * cy + -sr * -sy,
		cr * sp * sy + -sr * cy,
		cr * cp);
}

QAngle QAngle::Normalize() const
{
	QAngle angles(x, y, z);
	for (int i = 0; i < 3; ++i)
	{
		if (angles[i] < -180.0f)
			angles[i] += 360.0f;

		if (angles[i] > 180.0f)
			angles[i] -= 360.0f;
	}

	return std::move(angles);
}

QAngle QAngle::Clamp() const
{
	QAngle angles(x, y, z);

	if (angles.x > 89.0f && angles.x <= 180.0f)
		angles.x = 89.0f;

	if (angles.x > 180.0f)
		angles.x = angles.x - 360.0f;

	if (angles.x < -89.0f)
		angles.x = -89.0f;

	angles.y = fmod(angles.y + 180.0f, 360.0f) - 180.0f;
	angles.z = 0;

	return std::move(angles);
}

float QAngle::operator[](int index) const
{
	if (index <= 0)
		return x;
	if (index == 1)
		return y;
	return z;
}

float & QAngle::operator[](int index)
{
	if (index <= 0)
		return x;
	if (index == 1)
		return y;
	return z;
}

bool QAngle::operator==(const QAngle & v) const
{
	return (x == v.x && y == v.y && z == v.z);
}

bool QAngle::operator!=(const QAngle & v) const
{
	return (x != v.x || y != v.y || z != v.z);
}

QAngle QAngle::operator+(const QAngle & v) const
{
	return QAngle(x + v.x, y + v.y, z + v.z);
}

QAngle QAngle::operator-(const QAngle & v) const
{
	return QAngle(x - v.x, y - v.y, z - v.z);
}

QAngle QAngle::operator*(const QAngle & v) const
{
	return QAngle(x * v.x, y * v.y, z * v.z);
}

QAngle QAngle::operator/(const QAngle & v) const
{
	return QAngle(x / v.x, y / v.y, z / v.z);
}

QAngle QAngle::operator+(const vec_t & f) const
{
	return QAngle(x + f, y + f, z + f);
}

QAngle QAngle::operator-(const vec_t & f) const
{
	return QAngle(x - f, y - f, z - f);
}

QAngle QAngle::operator*(const vec_t & f) const
{
	return QAngle(x * f, y * f, z * f);
}

QAngle QAngle::operator/(const vec_t & f) const
{
	return QAngle(x / f, y / f, z / f);
}

QAngle QAngle::operator-() const
{
	return QAngle(-x, -y, -z);
}

QAngle & QAngle::operator+=(const QAngle & v)
{
	x += v.x;
	y += v.y;
	z += v.z;
	return *this;
}

QAngle & QAngle::operator-=(const QAngle & v)
{
	x -= v.x;
	y -= v.y;
	z -= v.z;
	return *this;
}

QAngle & QAngle::operator*=(const QAngle & v)
{
	x *= v.x;
	y *= v.y;
	z *= v.z;
	return *this;
}

QAngle & QAngle::operator/=(const QAngle & v)
{
	x /= v.x;
	y /= v.y;
	z /= v.z;
	return *this;
}

QAngle & QAngle::operator+=(const vec_t & f)
{
	x += f;
	y += f;
	z += f;
	return *this;
}

QAngle & QAngle::operator-=(const vec_t & f)
{
	x -= f;
	y -= f;
	z -= f;
	return *this;
}

QAngle & QAngle::operator*=(const vec_t & f)
{
	x *= f;
	y *= f;
	z *= f;
	return *this;
}

QAngle & QAngle::operator/=(const vec_t & f)
{
	x /= f;
	y /= f;
	z /= f;
	return *this;
}

Vector2D::Vector2D(vec_t x, vec_t y) : x(x), y(y)
{
}

Vector4D::Vector4D(vec_t x, vec_t y, vec_t z, vec_t w) : x(x), y(y), z(z), w(w)
{
}
