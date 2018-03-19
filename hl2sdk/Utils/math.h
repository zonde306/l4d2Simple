#pragma once
#include "../../l4d2Simple2/vector.h"
#include "../Structs/matrix.h"

namespace math
{
	void VectorTransform(const Vector& in1, const matrix3x4_t &in2, Vector &out);
	QAngle CalculateAim(const Vector &origin, const Vector &target);
	float GetAnglesFieldOfView(const QAngle& myAngles, const QAngle& aimAngles);
	float GetVectorDistance(const Vector& origin1, const Vector& origin2, bool squared = false);
	bool WorldToScreen(const Vector& origin, Vector& screen);
};
