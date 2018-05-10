#pragma once
#include <d3d9.h>
#include <d3dx9.h>
#include "../Interfaces/IVModelInfo.h"
#include "../../l4d2Simple2/vector.h"
#include "../Structs/matrix.h"
#include "../Structs/usercmd.h"

class CBasePlayer;

namespace math
{
	void VectorTransform(const Vector& in1, const matrix3x4_t &in2, Vector &out);
	QAngle CalculateAim(const Vector &origin, const Vector &target);
	float GetAnglesFieldOfView(const QAngle& myAngles, const QAngle& aimAngles);
	float GetVectorDistance(const Vector& origin1, const Vector& origin2, bool squared = false);

	bool WorldToScreen(const Vector& origin, Vector& screen);
	bool WorldToScreenEx(const Vector& origin, Vector& screen);
	bool ScreenTransform(const Vector& origin, Vector& screen);
	bool WorldToScreen(const D3DXVECTOR3& origin, D3DXVECTOR3& screen);

	void CorrectMovement(Vector vOldAngles, CUserCmd* pCmd, Vector Viewangs);
	void CorrectMovement(const QAngle& vOldAngles, CUserCmd* pCmd, float fOldForward, float fOldSidemove);

	float VectorNormalize(Vector& v);
	void AngleNormalize(QAngle& angles);
	void ClampAngles(QAngle& angles);
	void AngleVectors(const QAngle &angles, Vector& forward);
	void AngleVectors(const QAngle &angles, Vector& forward, Vector& right, Vector& up);
	void AngleVectorsTranspose(const QAngle &angles, Vector *forward, Vector *right, Vector *up);
	void VectorAngles(const Vector& forward, QAngle &angles);
	void VectorAngles(const Vector & forward, const Vector & pseudoup, QAngle & angles);
	void CrossProduct(const Vector& v1, const Vector& v2, Vector& cross);
	void MatrixGetColumn(const matrix3x4_t& in, int column, Vector &out);
	void MatrixSetColumn(const Vector &in, int column, matrix3x4_t& out);
	void SetIdentityMatrix(matrix3x4_t& matrix);
	float QuaternionNormalize(Quaternion &q);
	void MatrixAngles(const matrix3x4_t &matrix, Quaternion &q, Vector &pos);
	void VectorVectors(const Vector & forward, Vector & right, Vector & up);
	Vector CrossProduct(const Vector& a, const Vector& b);
	Vector DoEnemyCircle(const Vector &vecDelta, float *flRotation = nullptr);
	float GetDelta(float hiSpeed, float maxSpeed, float airAcceleRate);
	float DotProduct(const Vector & a, const Vector & b);
	Vector VelocityExtrapolate(const Vector& origin, const Vector& velocity, bool forwardtrack = false);

	void VectorRotate(const Vector &in1, const matrix3x4_t& in2, Vector &out);
	void VectorRotate(const Vector &in1, const QAngle &in2, Vector &out);
	void AngleMatrix(const QAngle &angles, matrix3x4_t& matrix);

	// Ray 到 AABB 相交检测
	bool IntersectRayWithAABB(const Vector& origin, const Vector& dir, const Vector& min, const Vector& max);

	// Ray 到 Capsule 相交检测
	bool DoesIntersectCapsule(const Vector& eyePos, const Vector& myDir, const Vector& capsuleA, const Vector& capsuleB, float radius);

	// Ray 到 OBB 相交检测
	bool InsersectRayWithOBB(CBasePlayer* local, CBasePlayer* target, mstudiobbox_t* hitbox, const QAngle& viewangles);
};
