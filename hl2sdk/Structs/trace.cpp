#include "trace.h"

inline void Ray_t::Init(const Vector & vecStart, const Vector & vecEnd)
{
	m_Delta = vecEnd - vecStart;

	m_IsSwept = (m_Delta.LengthSqr() != 0);

	m_Extents.x = m_Extents.y = m_Extents.z = 0.0f;
	m_IsRay = true;

	m_StartOffset.x = m_StartOffset.y = m_StartOffset.z = 0.0f;

	m_Start = vecStart;
}

inline void Ray_t::Init(Vector const & start, Vector const & end, Vector const & mins, Vector const & maxs)
{
	Assert(&end);
	m_Delta = end - start;

	m_IsSwept = (m_Delta.LengthSqr() != 0);

	m_Extents = maxs - mins;
	m_Extents *= 0.5f;
	m_IsRay = (m_Extents.LengthSqr() < 1e-6);

	// Offset m_Start to be in the center of the box...
	m_StartOffset = mins + maxs;
	m_StartOffset *= 0.5f;
	m_Start = start + m_StartOffset;
	m_StartOffset *= -1.0f;
}

// compute inverse delta

inline Vector Ray_t::InvDelta() const
{
	Vector vecInvDelta;
	for (int iAxis = 0; iAxis < 3; ++iAxis)
	{
		if (m_Delta[iAxis] != 0.0f)
		{
			vecInvDelta[iAxis] = 1.0f / m_Delta[iAxis];
		}
		else
		{
			vecInvDelta[iAxis] = FLT_MAX;
		}
	}
	return vecInvDelta;
}

inline bool CTraceFilter::ShouldHitEntity(CBaseEntity * pEntityHandle, int contentsMask)
{
	return !(pEntityHandle == pSkip1);
}

inline TraceType_t CTraceFilter::GetTraceType() const
{
	return TRACE_EVERYTHING;
}

inline CTraceFilterSimple::CTraceFilterSimple(const CBaseEntity * passentity, int collisionGroup)
{
	m_pPassEnt = passentity;
	m_collisionGroup = collisionGroup;
}

inline bool CTraceFilterSimple::ShouldHitEntity(CBaseEntity * pHandleEntity, int contentsMask)
{
	return !(pHandleEntity == m_pPassEnt);
}

inline void CTraceFilterSimple::SetPassEntity(const CBaseEntity * pPassEntity) { m_pPassEnt = pPassEntity; }

inline void CTraceFilterSimple::SetCollisionGroup(int iCollisionGroup) { m_collisionGroup = iCollisionGroup; }

inline const CBaseEntity * CTraceFilterSimple::GetPassEntity(void) { return m_pPassEnt; }

inline CTraceListData::CTraceListData(int nLeafMax, int nEntityMax)
{
	// MEM_ALLOC_CREDIT();
	m_nLeafCount = 0;
	m_aLeafList.SetSize(nLeafMax);

	m_nEntityCount = 0;
	m_aEntityList.SetSize(nEntityMax);
}

inline CTraceListData::~CTraceListData()
{
	m_nLeafCount = 0;
	m_aLeafList.RemoveAll();

	m_nEntityCount = 0;
	m_aEntityList.RemoveAll();
}

inline void CTraceListData::Reset(void)
{
	m_nLeafCount = 0;
	m_nEntityCount = 0;
}

// For leaves...

inline void CTraceListData::AddLeaf(int iLeaf)
{
	if (m_nLeafCount >= m_aLeafList.Count())
	{
		// DevMsg("CTraceListData: Max leaf count along ray exceeded!\n");
		m_aLeafList.AddMultipleToTail(m_aLeafList.Count());
	}

	m_aLeafList[m_nLeafCount] = iLeaf;
	m_nLeafCount++;
}

// For entities...

inline IterationRetval_t CTraceListData::EnumElement(IHandleEntity * pHandleEntity)
{
	if (m_nEntityCount >= m_aEntityList.Count())
	{
		// DevMsg("CTraceListData: Max entity count along ray exceeded!\n");
		m_aEntityList.AddMultipleToTail(m_aEntityList.Count());
	}

	m_aEntityList[m_nEntityCount] = pHandleEntity;
	m_nEntityCount++;

	return ITERATION_CONTINUE;
}
