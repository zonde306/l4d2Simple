#pragma once
#include "handle.h"
#include "baseentity.h"
#include "../definitions.h"
#include "../../l4d2Simple2/vector.h"
#include "../Utils/utlvector.h"
#include <cfloat>
#include <windows.h>

class __declspec(align(16))VectorAligned : public Vector
{
public:
	inline VectorAligned & operator=(const Vector &vOther)
	{
		Init(vOther.x, vOther.y, vOther.z);
		w = 0.f;
		return *this;
	}

	// this space is used anyway
	float w;
};


struct Ray_t
{
	VectorAligned  m_Start;    // starting point, centered within the extents
	VectorAligned  m_Delta;    // direction + length of the ray
	VectorAligned  m_StartOffset;    // Add this to m_Start to get the actual ray start
	VectorAligned  m_Extents;    // Describes an axis aligned box extruded along a ray

								 // 要不要都可以的
								 // const matrix3x4_t *m_pWorldAxisTransform;

	bool    m_IsRay;    // are the extents zero?
	bool    m_IsSwept;    // is delta != 0?

	void Init(const Vector& vecStart, const Vector& vecEnd);

	void Init(Vector const& start, Vector const& end, Vector const& mins, Vector const& maxs);

	// compute inverse delta
	Vector InvDelta() const;

private:
};

struct csurface_t
{
	const char *name;
	short surfaceProps;
	unsigned short flags;
};

struct cplane_t
{
	Vector normal;
	float dist;
	byte type;
	byte signbits;
	byte pad[2];
};

struct trace_t
{
	Vector start;
	Vector end;
	cplane_t plane;
	float fraction;
	int contents;
	WORD dispFlags;
	bool allsolid;
	bool startSolid;
	float fractionLeftSolid;
	csurface_t surface;
	int hitGroup;
	short physicsBone;
	CBaseEntity* m_pEnt;
	int hitbox;
};

enum TraceType_t
{
	TRACE_EVERYTHING = 0,
	TRACE_WORLD_ONLY,				// NOTE: This does *not* test static props!!!
	TRACE_ENTITIES_ONLY,			// NOTE: This version will *not* test static props
	TRACE_EVERYTHING_FILTER_PROPS,	// NOTE: This version will pass the IHandleEntity for props through the filter, unlike all other filters
};

class ITraceFilter
{
public:
	virtual bool			ShouldHitEntity(CBaseEntity* pEntity, int mask) = 0;
	virtual TraceType_t		GetTraceType() const = 0;
};

class CTraceFilter : public ITraceFilter
{
public:
	virtual bool ShouldHitEntity(CBaseEntity* pEntityHandle, int contentsMask) override;

	virtual TraceType_t GetTraceType() const override;

	CBaseEntity* pSkip1;
};

class CTraceFilterSimple : public CTraceFilter
{
public:
	CTraceFilterSimple(const CBaseEntity* passentity, int collisionGroup);

	virtual bool ShouldHitEntity(CBaseEntity* pHandleEntity, int contentsMask) override;
	virtual void SetPassEntity(const CBaseEntity* pPassEntity);
	virtual void SetCollisionGroup(int iCollisionGroup);

	const CBaseEntity *GetPassEntity(void);

private:
	const CBaseEntity *m_pPassEnt;
	int m_collisionGroup;
};

enum IterationRetval_t
{
	ITERATION_CONTINUE = 0,
	ITERATION_STOP,
};

class IPartitionEnumerator
{
public:
	virtual IterationRetval_t EnumElement(IHandleEntity *pHandleEntity) = 0;
};

#define TLD_DEF_LEAF_MAX	256
#define TLD_DEF_ENTITY_MAX	1024

class CTraceListData : public IPartitionEnumerator
{
public:
	CTraceListData(int nLeafMax = TLD_DEF_LEAF_MAX, int nEntityMax = TLD_DEF_ENTITY_MAX);

	~CTraceListData();

	void Reset(void);

	inline bool	IsEmpty(void) const
	{
		return (m_nLeafCount == 0 && m_nEntityCount == 0);
	}

	inline int		LeafCount(void) const { return m_nLeafCount; }
	inline int		LeafCountMax(void) const { return m_aLeafList.Count(); }
	inline void    LeafCountReset(void) { m_nLeafCount = 0; }

	inline int		EntityCount(void) const { return m_nEntityCount; }
	inline int		EntityCountMax(void) const { return m_aEntityList.Count(); }
	inline void		EntityCountReset(void) { m_nEntityCount = 0; }

	// For leaves...
	void AddLeaf(int iLeaf);

	// For entities...
	IterationRetval_t EnumElement(IHandleEntity *pHandleEntity);

public:

	int							m_nLeafCount;
	CUtlVector<int>				m_aLeafList;

	int							m_nEntityCount;
	CUtlVector<IHandleEntity*>	m_aEntityList;
};

