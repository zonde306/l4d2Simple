#pragma once
#include "../definitions.h"
#include "../../l4d2Simple2/vector.h"

typedef int SideType;

// Used to represent sides of things like planes.
#define	SIDE_FRONT	0
#define	SIDE_BACK	1
#define	SIDE_ON		2

#define VP_EPSILON	0.01f

class VPlane
{
public:
	VPlane();
	VPlane(const Vector &vNormal, vec_t dist);

	void		Init(const Vector &vNormal, vec_t dist);

	// Return the distance from the point to the plane.
	vec_t		DistTo(const Vector &vVec) const;

	// Copy.
	VPlane&		operator=(const VPlane &thePlane);

	// Returns SIDE_ON, SIDE_FRONT, or SIDE_BACK.
	// The epsilon for SIDE_ON can be passed in.
	SideType	GetPointSide(const Vector &vPoint, vec_t sideEpsilon = VP_EPSILON) const;

	// Returns SIDE_FRONT or SIDE_BACK.
	SideType	GetPointSideExact(const Vector &vPoint) const;

	// Classify the box with respect to the plane.
	// Returns SIDE_ON, SIDE_FRONT, or SIDE_BACK
	SideType	BoxOnPlaneSide(const Vector &vMin, const Vector &vMax) const;

#ifndef VECTOR_NO_SLOW_OPERATIONS
	// Flip the plane.
	VPlane		Flip();

	// Get a point on the plane (normal*dist).
	Vector		GetPointOnPlane() const;

	// Snap the specified point to the plane (along the plane's normal).
	Vector		SnapPointToPlane(const Vector &vPoint) const;
#endif

public:
	Vector		m_Normal;
	vec_t		m_Dist;

#ifdef VECTOR_NO_SLOW_OPERATIONS
private:
	// No copy constructors allowed if we're in optimal mode
	VPlane(const VPlane& vOther);
#endif
};

//-----------------------------------------------------------------------------
// Utility to simplify table construction. No constructor means can use
// traditional C-style initialization
//-----------------------------------------------------------------------------
class TableVector
{
public:
	vec_t x, y, z;

	operator Vector &() { return *((Vector *)(this)); }
	operator const Vector &() const { return *((const Vector *)(this)); }

	// array access...
	inline vec_t& operator[](int i)
	{
		Assert((i >= 0) && (i < 3));
		return ((vec_t*)this)[i];
	}

	inline vec_t operator[](int i) const
	{
		Assert((i >= 0) && (i < 3));
		return ((vec_t*)this)[i];
	}
};
