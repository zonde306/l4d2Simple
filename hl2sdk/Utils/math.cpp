#include "math.h"
#include "../interfaces.h"
#include "../../l4d2Simple2/dx9hook.h"
#include "../Structs/baseplayer.h"
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

enum
{
	PITCH = 0,	// up / down
	YAW,		// left / right
	ROLL		// fall over
};

const VMatrix* g_pWorldToScreenMatrix = nullptr;

// Math routines done in optimized assembly math package routines
void inline SinCos(float radians, float *sine, float *cosine)
{
	*sine = sin(radians);
	*cosine = cos(radians);
}

void math::VectorTransform(const Vector & in1, const matrix3x4_t & in2, Vector & out)
{
	out[0] = in1.Dot(Vector(in2[0][0], in2[0][1], in2[0][2])) + in2[0][3];
	out[1] = in1.Dot(Vector(in2[1][0], in2[1][1], in2[1][2])) + in2[1][3];
	out[2] = in1.Dot(Vector(in2[2][0], in2[2][1], in2[2][2])) + in2[2][3];
}

QAngle math::CalculateAim(const Vector & origin, const Vector & target)
{
	/*
	Vector angles;
	Vector deltaPos = target - origin;

	angles.y = atan2(deltaPos.y, deltaPos.x) * 180 / M_PI;
	angles.x = atan2(-(deltaPos.z), sqrt(deltaPos.x * deltaPos.x + deltaPos.y * deltaPos.y)) * 180 / M_PI;
	angles.z = 0.0f;

	return angles.Normalize();
	*/

	return (target - origin).Normalize().toAngles();
}

float math::GetAnglesFieldOfView(const QAngle & myAngles, const QAngle & aimAngles)
{
	/*
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
	*/

	Vector vAngles = myAngles.Forward();
	Vector vAimAngles = aimAngles.Forward();
	return RAD2DEG(acos(vAngles.Dot(vAimAngles) / vAngles.LengthSqr()));
}

float math::GetVectorDistance(const Vector & origin1, const Vector & origin2, bool squared)
{
	if (squared)
		return (origin1 - origin2).Length();

	return (origin1 - origin2).LengthSqr();
}

bool math::WorldToScreen(const Vector & origin, Vector & screen)
{
	if (!origin.IsValid())
		return false;

	int m_iWidth, m_iHeight;
	g_pInterface->Engine->GetScreenSize(m_iWidth, m_iHeight);

	if (g_pWorldToScreenMatrix == nullptr)
		g_pWorldToScreenMatrix = &g_pInterface->Engine->WorldToScreenMatrix();

	float w = (*g_pWorldToScreenMatrix)[3][0] * origin[0] +
		(*g_pWorldToScreenMatrix)[3][1] * origin[1] +
		(*g_pWorldToScreenMatrix)[3][2] * origin[2] +
		(*g_pWorldToScreenMatrix)[3][3];

	screen.z = 0;
	if (w > 0.01f)
	{
		float inverseWidth = 1 / w;
		screen.x = m_iWidth / 2 + (0.5f * ((
			(*g_pWorldToScreenMatrix)[0][0] * origin[0] +
			(*g_pWorldToScreenMatrix)[0][1] * origin[1] +
			(*g_pWorldToScreenMatrix)[0][2] * origin[2] +
			(*g_pWorldToScreenMatrix)[0][3]) * inverseWidth) * m_iWidth + 0.5f);

		screen.y = m_iHeight / 2 - (0.5f * ((
			(*g_pWorldToScreenMatrix)[1][0] * origin[0] +
			(*g_pWorldToScreenMatrix)[1][1] * origin[1] +
			(*g_pWorldToScreenMatrix)[1][2] * origin[2] +
			(*g_pWorldToScreenMatrix)[1][3]) * inverseWidth) * m_iHeight + 0.5f);

		return true;
	}
	
	return false;
}

inline void FindScreenPoint(Vector &point, int screenwidth, int screenheight, int degrees)
{
	float x2 = screenwidth / 2.0f;
	float y2 = screenheight / 2.0f;

	float d = sqrt(pow((point.x - x2), 2) + (pow((point.y - y2), 2))); //Distance
	float r = degrees / d; //Segment ratio

	point.x = r * point.x + (1 - r) * x2; //find point that divides the segment
	point.y = r * point.y + (1 - r) * y2; //into the ratio (1-r):r
}

bool math::WorldToScreenEx(const Vector & origin, Vector & screen)
{
	int iScreenWidth, iScreenHeight;
	g_pInterface->Engine->GetScreenSize(iScreenWidth, iScreenHeight);

	bool st = ScreenTransform(origin, screen);
	screen.x = (iScreenWidth / 2.0f) + (screen.x * iScreenWidth) / 2.0f;
	screen.y = (iScreenHeight / 2.0f) - (screen.y * iScreenHeight) / 2.0f;

	/*
	float x = iScreenWidth / 2.0f;
	float y = iScreenHeight / 2.0f;
	x += 0.5f * screen.x * iScreenWidth + 0.5f;
	y -= 0.5f * screen.y * iScreenHeight + 0.5f;
	screen.x = x;
	screen.y = y;
	*/

	if (screen.x > iScreenWidth || screen.x < 0.0f || screen.y > iScreenHeight || screen.y < 0.0f || !st)
	{
		FindScreenPoint(screen, iScreenWidth, iScreenHeight, iScreenHeight / 2);
		return false;
	}

	return true;
}

bool math::ScreenTransform(const Vector & origin, Vector & screen)
{
	float w;

	if (g_pWorldToScreenMatrix == nullptr)
		g_pWorldToScreenMatrix = &g_pInterface->Engine->WorldToScreenMatrix();

	screen.x = (*g_pWorldToScreenMatrix)[0][0] * origin[0] + (*g_pWorldToScreenMatrix)[0][1] *
		origin[1] + (*g_pWorldToScreenMatrix)[0][2] * origin[2] + (*g_pWorldToScreenMatrix)[0][3];

	screen.y = (*g_pWorldToScreenMatrix)[1][0] * origin[0] + (*g_pWorldToScreenMatrix)[1][1] *
		origin[1] + (*g_pWorldToScreenMatrix)[1][2] * origin[2] + (*g_pWorldToScreenMatrix)[1][3];

	w = (*g_pWorldToScreenMatrix)[3][0] * origin[0] + (*g_pWorldToScreenMatrix)[3][1] *
		origin[1] + (*g_pWorldToScreenMatrix)[3][2] * origin[2] + (*g_pWorldToScreenMatrix)[3][3];

	screen.z = 0.0f;

	if (w < 0.001f)
	{
		screen.x *= 100000;
		screen.y *= 100000;
		return false;
	}
	
	/*
	float inverseWidth = 1.0f / w;
	screen.x *= inverseWidth;
	screen.y *= inverseWidth;
	*/

	screen.x /= w;
	screen.y /= w;
	
	return true;
}

bool math::WorldToScreen(const D3DXVECTOR3 & origin, D3DXVECTOR3 & screen)
{
	IDirect3DDevice9* pDevice = g_pDirextXHook->GetDevice();
	if (pDevice == nullptr)
		return false;

	D3DVIEWPORT9 viewPort;
	D3DXVECTOR3 vOrthoLocation;
	D3DXMATRIX projection, view, world, identity;

	pDevice->GetTransform(D3DTS_VIEW, &view);
	pDevice->GetTransform(D3DTS_PROJECTION, &projection);
	pDevice->GetTransform(D3DTS_WORLD, &world);

	pDevice->GetViewport(&viewPort);
	D3DXMatrixIdentity(&identity);

	D3DXVec3Project(&screen, &origin, &viewPort, &projection, &view, &identity);
	D3DXVec3Unproject(&screen, &origin, &viewPort, &projection, &view, &identity);

	if (screen.x < 0.0f || screen.x > viewPort.Width ||
		screen.y < 0.0f || screen.y > viewPort.Height)
	{
		return false;
	}

	return true;
}

void math::CorrectMovement(Vector vOldAngles, CUserCmd * pCmd, Vector Viewangs)
{
	Vector vMove(pCmd->forwardmove, pCmd->sidemove, pCmd->upmove);
	float flSpeed = sqrt(vMove.x * vMove.x + vMove.y * vMove.y), flYaw;
	Vector vMove2 = vMove.toAngles();

	flYaw = DEG2RAD(Viewangs.y - vOldAngles.y + vMove2.y);
	pCmd->forwardmove = cos(flYaw) * flSpeed;
	pCmd->sidemove = sin(flYaw) * flSpeed;

	if (Viewangs.x < -90.f || Viewangs.x > 90.f)
		pCmd->forwardmove = -pCmd->forwardmove;
}

void math::CorrectMovement(const QAngle & vOldAngles, CUserCmd * pCmd, float fOldForward, float fOldSidemove)
{
	float deltaView = pCmd->viewangles[1] - vOldAngles[1];
	float f1;
	float f2;

	if (vOldAngles[1] < 0.f)
		f1 = 360.0f + vOldAngles[1];
	else
		f1 = vOldAngles[1];

	if (pCmd->viewangles[1] < 0.0f)
		f2 = 360.0f + pCmd->viewangles[1];
	else
		f2 = pCmd->viewangles[1];

	if (f2 < f1)
		deltaView = abs(f2 - f1);
	else
		deltaView = 360.0f - abs(f1 - f2);
	deltaView = 360.0f - deltaView;

	pCmd->forwardmove = cos(DEG2RAD(deltaView)) * fOldForward + cos(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
	pCmd->sidemove = sin(DEG2RAD(deltaView)) * fOldForward + sin(DEG2RAD(deltaView + 90.f)) * fOldSidemove;
}

float math::VectorNormalize(Vector & v)
{
	int		i;
	float	length;

	length = 0;
	for (i = 0; i< 3; i++)
		length += v[i] * v[i];
	length = sqrtf(length);

	for (i = 0; i< 3; i++)
		v[i] /= length;

	return length;
}

void math::AngleNormalize(QAngle & angles)
{
	for (int i = 0; i < 3; ++i)
	{
		if (angles[i] < -180.0f)
			angles[i] += 360.0f;

		if (angles[i] > 180.0f)
			angles[i] -= 360.0f;
	}
}

void math::ClampAngles(QAngle & angles)
{
	if (angles.x > 89.0f && angles.x <= 180.0f)
		angles.x = 89.0f;

	if (angles.x > 180.0f)
		angles.x = angles.x - 360.0f;

	if (angles.x < -89.0f)
		angles.x = -89.0f;

	angles.y = fmodf(angles.y + 180, 360) - 180;

	angles.z = 0;
}

void math::AngleVectors(const QAngle & angles, Vector & forward)
{
	// Assert(s_bMathlibInitialized);
	// Assert(forward);

	float	sp, sy, cp, cy;

	SinCos(DEG2RAD(angles[YAW]), &sy, &cy);
	SinCos(DEG2RAD(angles[PITCH]), &sp, &cp);

	forward.x = cp * cy;
	forward.y = cp * sy;
	forward.z = -sp;
}

void math::AngleVectors(const QAngle & angles, Vector & forward, Vector & right, Vector & up)
{
	// Assert(s_bMathlibInitialized);

	float sr, sp, sy, cr, cp, cy;

#ifdef _X360
	fltx4 radians, scale, sine, cosine;
	radians = LoadUnaligned3SIMD(angles.Base());
	scale = ReplicateX4(M_PI_F / 180.f);
	radians = MulSIMD(radians, scale);
	SinCos3SIMD(sine, cosine, radians);
	sp = SubFloat(sine, 0);	sy = SubFloat(sine, 1);	sr = SubFloat(sine, 2);
	cp = SubFloat(cosine, 0);	cy = SubFloat(cosine, 1);	cr = SubFloat(cosine, 2);
#else
	SinCos(DEG2RAD(angles[YAW]), &sy, &cy);
	SinCos(DEG2RAD(angles[PITCH]), &sp, &cp);
	SinCos(DEG2RAD(angles[ROLL]), &sr, &cr);
#endif

	if (forward.IsValid())
	{
		forward.x = cp * cy;
		forward.y = cp * sy;
		forward.z = -sp;
	}

	if (right.IsValid())
	{
		right.x = (-1 * sr*sp*cy + -1 * cr*-sy);
		right.y = (-1 * sr*sp*sy + -1 * cr*cy);
		right.z = -1 * sr*cp;
	}

	if (up.IsValid())
	{
		up.x = (cr*sp*cy + -sr * -sy);
		up.y = (cr*sp*sy + -sr * cy);
		up.z = cr * cp;
	}
}

void math::AngleVectorsTranspose(const QAngle & angles, Vector * forward, Vector * right, Vector * up)
{
	// Assert(s_bMathlibInitialized);
	float sr, sp, sy, cr, cp, cy;

	SinCos(DEG2RAD(angles[YAW]), &sy, &cy);
	SinCos(DEG2RAD(angles[PITCH]), &sp, &cp);
	SinCos(DEG2RAD(angles[ROLL]), &sr, &cr);

	if (forward)
	{
		forward->x = cp * cy;
		forward->y = (sr*sp*cy + cr * -sy);
		forward->z = (cr*sp*cy + -sr * -sy);
	}

	if (right)
	{
		right->x = cp * sy;
		right->y = (sr*sp*sy + cr * cy);
		right->z = (cr*sp*sy + -sr * cy);
	}

	if (up)
	{
		up->x = -sp;
		up->y = sr * cp;
		up->z = cr * cp;
	}
}

void math::VectorAngles(const Vector & forward, QAngle & angles)
{
	// Assert(s_bMathlibInitialized);
	float	tmp, yaw, pitch;

	if (forward[1] == 0 && forward[0] == 0)
	{
		yaw = 0;
		if (forward[2] > 0)
			pitch = 270;
		else
			pitch = 90;
	}
	else
	{
		yaw = (atan2(forward[1], forward[0]) * 180 / M_PI_F);
		if (yaw < 0)
			yaw += 360;

		tmp = sqrt(forward[0] * forward[0] + forward[1] * forward[1]);
		pitch = (atan2(-forward[2], tmp) * 180 / M_PI_F);
		if (pitch < 0)
			pitch += 360;
	}

	angles[0] = pitch;
	angles[1] = yaw;
	angles[2] = 0;
}

void math::VectorAngles(const Vector & forward, const Vector & pseudoup, QAngle & angles)
{
	Vector left;

	CrossProduct(pseudoup, forward, left);
	VectorNormalize(left);

	float xyDist = sqrtf(forward[0] * forward[0] + forward[1] * forward[1]);

	// enough here to get angles?
	if (xyDist > 0.001f)
	{
		// (yaw)	y = ATAN( forward.y, forward.x );		-- in our space, forward is the X axis
		angles[1] = RAD2DEG(atan2f(forward[1], forward[0]));

		// The engine does pitch inverted from this, but we always end up negating it in the DLL
		// UNDONE: Fix the engine to make it consistent
		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));

		float up_z = (left[1] * forward[0]) - (left[0] * forward[1]);

		// (roll)	z = ATAN( left.z, up.z );
		angles[2] = RAD2DEG(atan2f(left[2], up_z));
	}
	else	// forward is mostly Z, gimbal lock-
	{
		// (yaw)	y = ATAN( -left.x, left.y );			-- forward is mostly z, so use right for yaw
		angles[1] = RAD2DEG(atan2f(-left[0], left[1])); //This was originally copied from the "void MatrixAngles( const matrix3x4_t& matrix, float *angles )" code, and it's 180 degrees off, negated the values and it all works now (Dave Kircher)

		// The engine does pitch inverted from this, but we always end up negating it in the DLL
		// UNDONE: Fix the engine to make it consistent
		// (pitch)	x = ATAN( -forward.z, sqrt(forward.x*forward.x+forward.y*forward.y) );
		angles[0] = RAD2DEG(atan2f(-forward[2], xyDist));

		// Assume no roll in this case as one degree of freedom has been lost (i.e. yaw == roll)
		angles[2] = 0;
	}
}

void math::CrossProduct(const Vector & v1, const Vector & v2, Vector & cross)
{
	// Assert(s_bMathlibInitialized);
	// Assert(v1 != cross);
	// Assert(v2 != cross);
	cross[0] = v1[1] * v2[2] - v1[2] * v2[1];
	cross[1] = v1[2] * v2[0] - v1[0] * v2[2];
	cross[2] = v1[0] * v2[1] - v1[1] * v2[0];
}

void math::MatrixGetColumn(const matrix3x4_t & in, int column, Vector & out)
{
	out.x = in[0][column];
	out.y = in[1][column];
	out.z = in[2][column];
}

void math::MatrixSetColumn(const Vector & in, int column, matrix3x4_t & out)
{
	out[0][column] = in.x;
	out[1][column] = in.y;
	out[2][column] = in.z;
}

void math::SetIdentityMatrix(matrix3x4_t & matrix)
{
	memset(matrix.Base(), 0, sizeof(float) * 3 * 4);
	matrix[0][0] = 1.0;
	matrix[1][1] = 1.0;
	matrix[2][2] = 1.0;
}

float math::QuaternionNormalize(Quaternion & q)
{
	// Assert( s_bMathlibInitialized );
	float radius, iradius;

	// Assert(q.IsValid());

	radius = q[0] * q[0] + q[1] * q[1] + q[2] * q[2] + q[3] * q[3];

	if (radius) // > FLT_EPSILON && ((radius < 1.0f - 4*FLT_EPSILON) || (radius > 1.0f + 4*FLT_EPSILON))
	{
		radius = sqrt(radius);
		iradius = 1.0f / radius;
		q[3] *= iradius;
		q[2] *= iradius;
		q[1] *= iradius;
		q[0] *= iradius;
	}
	return radius;
}

void math::MatrixAngles(const matrix3x4_t & matrix, Quaternion & q, Vector & pos)
{
#ifdef _VPROF_MATHLIB
	VPROF_BUDGET("MatrixQuaternion", "Mathlib");
#endif
	float trace;
	trace = matrix[0][0] + matrix[1][1] + matrix[2][2] + 1.0f;
	if (trace > 1.0f + FLT_EPSILON)
	{
		// VPROF_INCREMENT_COUNTER("MatrixQuaternion A",1);
		q.x = (matrix[2][1] - matrix[1][2]);
		q.y = (matrix[0][2] - matrix[2][0]);
		q.z = (matrix[1][0] - matrix[0][1]);
		q.w = trace;
	}
	else if (matrix[0][0] > matrix[1][1] && matrix[0][0] > matrix[2][2])
	{
		// VPROF_INCREMENT_COUNTER("MatrixQuaternion B",1);
		trace = 1.0f + matrix[0][0] - matrix[1][1] - matrix[2][2];
		q.x = trace;
		q.y = (matrix[1][0] + matrix[0][1]);
		q.z = (matrix[0][2] + matrix[2][0]);
		q.w = (matrix[2][1] - matrix[1][2]);
	}
	else if (matrix[1][1] > matrix[2][2])
	{
		// VPROF_INCREMENT_COUNTER("MatrixQuaternion C",1);
		trace = 1.0f + matrix[1][1] - matrix[0][0] - matrix[2][2];
		q.x = (matrix[0][1] + matrix[1][0]);
		q.y = trace;
		q.z = (matrix[2][1] + matrix[1][2]);
		q.w = (matrix[0][2] - matrix[2][0]);
	}
	else
	{
		// VPROF_INCREMENT_COUNTER("MatrixQuaternion D",1);
		trace = 1.0f + matrix[2][2] - matrix[0][0] - matrix[1][1];
		q.x = (matrix[0][2] + matrix[2][0]);
		q.y = (matrix[2][1] + matrix[1][2]);
		q.z = trace;
		q.w = (matrix[1][0] - matrix[0][1]);
	}

	QuaternionNormalize(q);

#if 0
	// check against the angle version
	RadianEuler ang;
	MatrixAngles(matrix, ang);
	Quaternion test;
	AngleQuaternion(ang, test);
	float d = QuaternionDotProduct(q, test);
	Assert(fabs(d) > 0.99 && fabs(d) < 1.01);
#endif

	math::MatrixGetColumn(matrix, 3, pos);
}

void math::VectorVectors(const Vector & forward, Vector & right, Vector & up)
{
	Vector tmp;

	if (forward[0] == 0 && forward[1] == 0)
	{
		// pitch 90 degrees up/down from identity
		right[0] = 0;
		right[1] = -1;
		right[2] = 0;
		up[0] = -forward[2];
		up[1] = 0;
		up[2] = 0;
	}
	else
	{
		tmp[0] = 0; tmp[1] = 0; tmp[2] = 1.0;
		CrossProduct(forward, tmp, right);
		VectorNormalize(right);
		CrossProduct(right, forward, up);
		VectorNormalize(up);
	}
}

Vector math::CrossProduct(const Vector & a, const Vector & b)
{
	if (!a.IsValid() || !b.IsValid())
		throw std::exception(XorStr(u8"提供的 Vector 无效"));

	return Vector(a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x);
}

Vector math::DoEnemyCircle(const Vector & vecDelta, float * flRotation)
{
	float flRadius = 360.0f;
	int iScreenWidth, iScreenHeight;
	g_pInterface->Engine->GetScreenSize(iScreenWidth, iScreenHeight);

	QAngle vRealAngles;
	g_pInterface->Engine->GetViewAngles(vRealAngles);

	Vector vForward, vRight, vUp(0.0f, 0.0f, 1.0f);

	AngleVectors(vRealAngles, vForward);

	vForward.z = 0.0f;
	VectorNormalize(vForward);
	CrossProduct(vUp, vForward, vRight);

	float flFront = DotProduct(vecDelta, vForward);
	float flSide = DotProduct(vecDelta, vRight);
	float flXPosition = flRadius * -flSide;
	float flYPosition = flRadius * -flFront;

	float rotation = atan2(flXPosition, flYPosition) + M_PI_F;
	rotation *= 180.0f / M_PI_F;

	float flYawRadians = -(rotation)* M_PI_F / 180.0f;
	float flCosYaw = cos(flYawRadians);
	float flSinYaw = sin(flYawRadians);

	if (flRotation != nullptr)
		*flRotation = rotation;

	return Vector((iScreenWidth / 2.0f) + (flRadius * flSinYaw),
		(iScreenHeight / 2.0f) - (flRadius * flCosYaw), 0.0f);
}

float math::GetDelta(float hiSpeed, float maxSpeed, float airAcceleRate)
{
	float term = (30.0f - (airAcceleRate * maxSpeed / 66.0f)) / hiSpeed;
	if (term < 1.0f && term > -1.0f)
		return acos(term);

	return 0.0f;
}

float math::DotProduct(const Vector & a, const Vector & b)
{
	return (a.x * b.x + a.y * b.y + a.z * b.z);
}

Vector math::VelocityExtrapolate(const Vector & origin, const Vector & velocity, bool forwardtrack)
{
	int tick = 1;
	if (forwardtrack)
	{
		INetChannelInfo* netChan = g_pInterface->Engine->GetNetChannelInfo();
		if(netChan != nullptr)
			tick = TIME_TO_TICKS(netChan->GetLatency(NetFlow_Incoming) + netChan->GetLatency(NetFlow_Outgoing));
	}
	
	return origin + (velocity * (g_pInterface->GlobalVars->interval_per_tick * tick));
}

void math::VectorRotate(const Vector & in1, const matrix3x4_t & in2, Vector & out)
{
	out[0] = DotProduct(in1, in2[0]);
	out[1] = DotProduct(in1, in2[1]);
	out[2] = DotProduct(in1, in2[2]);
}

void math::VectorRotate(const Vector & in1, const QAngle & in2, Vector & out)
{
	matrix3x4_t matRotate;
	AngleMatrix(in2, matRotate);
	VectorRotate(in1, matRotate, out);
}

void math::AngleMatrix(const QAngle & angles, matrix3x4_t & matrix)
{
	float sr, sp, sy, cr, cp, cy;
	SinCos(DEG2RAD(angles[YAW]), &sy, &cy);
	SinCos(DEG2RAD(angles[PITCH]), &sp, &cp);
	SinCos(DEG2RAD(angles[ROLL]), &sr, &cr);

	// matrix = (YAW * PITCH) * ROLL
	matrix[0][0] = cp * cy;
	matrix[1][0] = cp * sy;
	matrix[2][0] = -sp;

	float crcy = cr * cy;
	float crsy = cr * sy;
	float srcy = sr * cy;
	float srsy = sr * sy;
	matrix[0][1] = sp * srcy - crsy;
	matrix[1][1] = sp * srsy + crcy;
	matrix[2][1] = sr * cp;

	matrix[0][2] = (sp*crcy + srsy);
	matrix[1][2] = (sp*crsy - srcy);
	matrix[2][2] = cr * cp;

	matrix[0][3] = 0.0f;
	matrix[1][3] = 0.0f;
	matrix[2][3] = 0.0f;
}

//credits to http://www.scratchapixel.com/ for the nice explanation of the algorithm and
//An Efficient and Robust Ray–Box Intersection Algorithm, Amy Williams et al. 2004.
//for inventing it :D
bool math::IntersectRayWithAABB(const Vector& origin, const Vector& dir, const Vector& min, const Vector& max)
{
	float tmin, tmax, tymin, tymax, tzmin, tzmax;
	
	if (dir.x >= 0)
	{
		tmin = (min.x - origin.x) / dir.x;
		tmax = (max.x - origin.x) / dir.x;
	}
	else
	{
		tmin = (max.x - origin.x) / dir.x;
		tmax = (min.x - origin.x) / dir.x;
	}

	if (dir.y >= 0)
	{
		tymin = (min.y - origin.y) / dir.y;
		tymax = (max.y - origin.y) / dir.y;
	}
	else
	{
		tymin = (max.y - origin.y) / dir.y;
		tymax = (min.y - origin.y) / dir.y;
	}

	if (tmin > tymax || tymin > tmax)
		return false;

	if (tymin > tmin)
		tmin = tymin;

	if (tymax < tmax)
		tmax = tymax;

	if (dir.z >= 0)
	{
		tzmin = (min.z - origin.z) / dir.z;
		tzmax = (max.z - origin.z) / dir.z;
	}
	else
	{
		tzmin = (max.z - origin.z) / dir.z;
		tzmax = (min.z - origin.z) / dir.z;
	}

	if (tmin > tzmax || tzmin > tmax)
		return false;

	//behind us
	if (tmin < 0 || tmax < 0)
		return false;

	return true;
}

#define SMALL_NUM 0.0f
bool math::DoesIntersectCapsule(const Vector & eyePos, const Vector & myDir, const Vector & capsuleA, const Vector & capsuleB, float radius)
{
	const static auto dist_Segment_to_Segment = [](const Vector& s1, const Vector& s2, const Vector& k1, const Vector& k2) -> float
	{
		Vector   u = s2 - s1;
		Vector   v = k2 - k1;
		Vector   w = s1 - k1;
		float    a = u.Dot(u);
		float    b = u.Dot(v);
		float    c = v.Dot(v);
		float    d = u.Dot(w);
		float    e = v.Dot(w);
		float    D = a * c - b * b;
		float    sc, sN, sD = D;
		float    tc, tN, tD = D;

		if (D < SMALL_NUM)
		{
			sN = 0.0;
			sD = 1.0;
			tN = e;
			tD = c;
		}
		else
		{
			sN = (b*e - c * d);
			tN = (a*e - b * d);
			if (sN < 0.0)
			{
				sN = 0.0;
				tN = e;
				tD = c;
			}
			else if (sN > sD)
			{
				sN = sD;
				tN = e + b;
				tD = c;
			}
		}

		if (tN < 0.0)
		{
			tN = 0.0;

			if (-d < 0.0)
				sN = 0.0;
			else if (-d > a)
				sN = sD;
			else {
				sN = -d;
				sD = a;
			}
		}
		else if (tN > tD)
		{
			tN = tD;

			if ((-d + b) < 0.0)
				sN = 0;
			else if ((-d + b) > a)
				sN = sD;
			else {
				sN = (-d + b);
				sD = a;
			}
		}

		sc = (abs(sN) < SMALL_NUM ? 0.0f : sN / sD);
		tc = (abs(tN) < SMALL_NUM ? 0.0f : tN / tD);

		Vector  dP = w + (u * sc) - (v * tc);
		return dP.Length();
	};

	Vector end = eyePos + (myDir * 8192.0f);
	auto dist = dist_Segment_to_Segment(eyePos, end, capsuleA, capsuleB);
	return dist < radius;
}

bool math::InsersectRayWithOBB(CBasePlayer * local, CBasePlayer * target, mstudiobbox_t * hitbox, const QAngle & viewangles)
{
	matrix3x4_t boneMatrix[128];
	if (!target->SetupBones(boneMatrix, 128, BONE_USED_BY_HITBOX, g_pInterface->GlobalVars->curtime))
		return false;
	
	Vector ray_start = local->GetEyePosition();
	Vector direction = ray_start.toAngles();
	matrix3x4_t BoneMatrix = boneMatrix[hitbox->bone];

	//Transform ray into model space of hitbox so we only have to deal with an AABB instead of OBB
	Vector ray_trans, dir_trans;
	VectorTransform(ray_start, BoneMatrix, ray_trans);
	VectorRotate(direction, BoneMatrix, dir_trans); //only rotate direction vector! no translation!

	return IntersectRayWithAABB(ray_trans, dir_trans, hitbox->bbmin, hitbox->bbmax);
}
