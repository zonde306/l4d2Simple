#include "vplane.h"


//-----------------------------------------------------------------------------
// Inlines.
//-----------------------------------------------------------------------------
inline VPlane::VPlane()
{
}

inline VPlane::VPlane(const Vector &vNormal, vec_t dist)
{
	m_Normal = vNormal;
	m_Dist = dist;
}

inline void	VPlane::Init(const Vector &vNormal, vec_t dist)
{
	m_Normal = vNormal;
	m_Dist = dist;
}

inline vec_t VPlane::DistTo(const Vector &vVec) const
{
	return vVec.Dot(m_Normal) - m_Dist;
}

inline VPlane& VPlane::operator=(const VPlane &thePlane)
{
	m_Normal = thePlane.m_Normal;
	m_Dist = thePlane.m_Dist;
	return *this;
}

#ifndef VECTOR_NO_SLOW_OPERATIONS

inline VPlane VPlane::Flip()
{
	return VPlane(-m_Normal, -m_Dist);
}

inline Vector VPlane::GetPointOnPlane() const
{
	return m_Normal * m_Dist;
}

inline Vector VPlane::SnapPointToPlane(const Vector &vPoint) const
{
	return vPoint - m_Normal * DistTo(vPoint);
}

#endif

inline SideType VPlane::GetPointSide(const Vector &vPoint, vec_t sideEpsilon) const
{
	vec_t fDist;

	fDist = DistTo(vPoint);
	if (fDist >= sideEpsilon)
		return SIDE_FRONT;
	else if (fDist <= -sideEpsilon)
		return SIDE_BACK;
	else
		return SIDE_ON;
}

inline SideType VPlane::GetPointSideExact(const Vector &vPoint) const
{
	return DistTo(vPoint) > 0.0f ? SIDE_FRONT : SIDE_BACK;
}


// BUGBUG: This should either simply use the implementation in mathlib or cease to exist.
// mathlib implementation is much more efficient.  Check to see that VPlane isn't used in
// performance critical code.
inline SideType VPlane::BoxOnPlaneSide(const Vector &vMin, const Vector &vMax) const
{
	int i, firstSide, side;
	TableVector vPoints[8] =
	{
		{ vMin.x, vMin.y, vMin.z },
		{ vMin.x, vMin.y, vMax.z },
		{ vMin.x, vMax.y, vMax.z },
		{ vMin.x, vMax.y, vMin.z },

		{ vMax.x, vMin.y, vMin.z },
		{ vMax.x, vMin.y, vMax.z },
		{ vMax.x, vMax.y, vMax.z },
		{ vMax.x, vMax.y, vMin.z },
	};

	firstSide = GetPointSideExact(vPoints[0]);
	for (i = 1; i < 8; i++)
	{
		side = GetPointSideExact(vPoints[i]);

		// Does the box cross the plane?
		if (side != firstSide)
			return SIDE_ON;
	}

	// Ok, they're all on the same side, return that.
	return firstSide;
}
