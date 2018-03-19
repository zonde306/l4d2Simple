#include "math.h"
#include "../interfaces.h"
#include <cmath>

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

#ifndef FLOAT32_NAN_BITS
#define FLOAT32_NAN_BITS	(unsigned long)0x7FC00000
#define FLOAT32_NAN			BitsToFloat(FLOAT32_NAN_BITS)
#define VEC_T_NAN			FLOAT32_NAN
#endif

void math::VectorTransform(const Vector & in1, const matrix3x4_t & in2, Vector & out)
{
	out[0] = in1.Dot(Vector(in2[0][0], in2[0][1], in2[0][2])) + in2[0][3];
	out[1] = in1.Dot(Vector(in2[1][0], in2[1][1], in2[1][2])) + in2[1][3];
	out[2] = in1.Dot(Vector(in2[2][0], in2[2][1], in2[2][2])) + in2[2][3];
}

QAngle math::CalculateAim(const Vector & origin, const Vector & target)
{
	Vector angles;
	Vector deltaPos = target - origin;

	angles.y = atan2(deltaPos.y, deltaPos.x) * 180 / M_PI;
	angles.x = atan2(-(deltaPos.z), sqrt(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y)) * 180 / M_PI;
	angles.z = 0.0f;

	return angles.Normalize();
}

float math::GetAnglesFieldOfView(const QAngle & myAngles, const QAngle & aimAngles)
{
	static auto getFov = [](float orgViewX, float orgViewY, float ViewX, float ViewY, float& fovX, float& fovY) -> void
	{
		fovX = std::abs(orgViewX - ViewX);
		fovY = std::abs(orgViewY - ViewY);

		if (fovY < -180) fovY += 360;

		if (fovY > 180) fovY -= 360;
	};

	static auto getDistance = [](float Xhere, float Yhere) -> float
	{
		Xhere = std::abs(Xhere);
		Yhere = std::abs(Yhere);

		Xhere *= Xhere;
		Yhere *= Yhere;
		float combined = (Xhere + Yhere);
		return sqrt(combined);
	};

	Vector toHim(0.0f, 0.0f, 0.0f);
	getFov(myAngles.x, myAngles.y, aimAngles.x, aimAngles.y, toHim.x, toHim.y);
	return getDistance(toHim.x, toHim.y);
}

float math::GetVectorDistance(const Vector & origin1, const Vector & origin2, bool squared)
{
	if (squared)
		return (origin1 - origin2).Length();

	return (origin1 - origin2).LengthSqr();
}

bool math::WorldToScreen(const Vector & origin, Vector & screen)
{
	if (origin.IsValid())
		return false;
	
	int width = 0, height = 0;
	g_pClientInterface->Engine->GetScreenSize(width, height);
	const VMatrix& w2sMatrix = g_pClientInterface->Engine->WorldToScreenMatrix();

	float w = w2sMatrix[3][0] * origin[0] + w2sMatrix[3][1] * origin[1] +
		w2sMatrix[3][2] * origin[2] + w2sMatrix[3][3];

	screen.z = 0.0f;

	if (w > 0.01f)
	{
		float w1 = 1 / w;

		screen.x = width / 2 + (0.5f * ((w2sMatrix[0][0] * origin[0] +
			w2sMatrix[0][1] * origin[1] + w2sMatrix[0][2] * origin[2] +
			w2sMatrix[0][3]) * w1) * width + 0.5);

		screen.y = height / 2 - (0.5f * ((w2sMatrix[1][0] * origin[0] +
			w2sMatrix[1][1] * origin[1] + w2sMatrix[1][2] * origin[2] +
			w2sMatrix[1][3]) * w1) * height + 0.5);

		return true;
	}

	return false;
}

