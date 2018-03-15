#pragma once
#include "IClientRenderable.h"
#include "../Structs/matrix.h"
#include "../Structs/baseentity.h"
#include "../Structs/trace.h"
#include "../Structs/color.h"

struct model_t;
class CStudioHdr;

FORWARD_DECLARE_HANDLE(LightCacheHandle_t);

struct ModelRenderInfo_t
{
	Vector origin;
	QAngle angles;
	CBaseEntity *pRenderable;
	const model_t *pModel;
	const matrix3x4_t *pModelToWorld;
	const matrix3x4_t *pLightingOffset;
	const Vector *pLightingOrigin;
	int flags;
	int entity_index;
	int skin;
	int body;
	int hitboxset;
	unsigned short instance;
	inline ModelRenderInfo_t()
	{
		pModelToWorld = NULL;
		pLightingOffset = NULL;
		pLightingOrigin = NULL;
	}
};

//-----------------------------------------------------------------------------
// What kind of material override is it?
//-----------------------------------------------------------------------------
enum OverrideType_t
{
	OVERRIDE_NORMAL = 0,
	OVERRIDE_BUILD_SHADOWS,
	OVERRIDE_DEPTH_WRITE,
	OVERRIDE_SSAO_DEPTH_WRITE,
};

struct studiohdr_t;
struct studiohwdata_t;

enum LightType_t
{
	MATERIAL_LIGHT_DISABLE = 0,
	MATERIAL_LIGHT_POINT,
	MATERIAL_LIGHT_DIRECTIONAL,
	MATERIAL_LIGHT_SPOT,
};

struct LightDesc_t
{
	LightType_t		m_Type;
	Vector			m_Color;
	Vector	m_Position;
	Vector  m_Direction;
	float   m_Range;
	float   m_Falloff;
	float   m_Attenuation0;
	float   m_Attenuation1;
	float   m_Attenuation2;
	float   m_Theta;
	float   m_Phi;
	// These aren't used by DX8. . used for software lighting.
	float	m_ThetaDot;
	float	m_PhiDot;
	unsigned int	m_Flags;


	LightDesc_t() {}

private:
	// No copy constructors allowed
	LightDesc_t(const LightDesc_t& vOther);
};

DECLARE_POINTER_HANDLE(StudioDecalHandle_t);
#define STUDIORENDER_DECAL_INVALID  ( (StudioDecalHandle_t)0 )

struct ColorTexelsInfo_t
{
	int						m_nWidth;
	int						m_nHeight;
	int						m_nMipmapCount;
	ImageFormat				m_ImageFormat;
	int						m_nByteCount;
	byte*					m_pTexelData;
};

class IMesh;
class IPooledVBAllocator;

struct ColorMeshInfo_t
{
	// A given color mesh can own a unique Mesh, or it can use a shared Mesh
	// (in which case it uses a sub-range defined by m_nVertOffset and m_nNumVerts)
	IMesh				*	m_pMesh;
	IPooledVBAllocator	*	m_pPooledVBAllocator;
	int						m_nVertOffsetInBytes;
	int						m_nNumVerts;
	ITexture			*   m_pLightmap;
	ColorTexelsInfo_t   *   m_pLightmapData;
};

struct DrawModelInfo_t
{
	studiohdr_t		*m_pStudioHdr;
	studiohwdata_t	*m_pHardwareData;
	StudioDecalHandle_t m_Decals;
	int				m_Skin;
	int				m_Body;
	int				m_HitboxSet;
	void			*m_pClientEntity;
	int				m_Lod;
	ColorMeshInfo_t	*m_pColorMeshes;
	bool			m_bStaticLighting;
	Vector			m_vecAmbientCube[6];		// ambient, and lights that aren't in locallight[]
	int				m_nLocalLightCount;
	LightDesc_t		m_LocalLightDescs[4];
};

enum
{
	ADDDECAL_TO_ALL_LODS = -1
};

//-----------------------------------------------------------------------------
// Model rendering state
//-----------------------------------------------------------------------------
struct DrawModelState_t
{
	studiohdr_t*			m_pStudioHdr;
	studiohwdata_t*			m_pStudioHWData;
	IClientRenderable*		m_pRenderable;
	const matrix3x4_t		*m_pModelToWorld;
	StudioDecalHandle_t		m_decals;
	int						m_drawFlags;
	int						m_lod;
};

struct StaticPropRenderInfo_t
{
	const matrix3x4_t		*pModelToWorld;
	const model_t			*pModel;
	IClientRenderable		*pRenderable;
	Vector					*pLightingOrigin;
	short					skin;
	ModelInstanceHandle_t	instance;
};

class IVModelRender
{
public:
#ifdef _CSGO
	void ForcedMaterialOverride(IMaterial *mat);
	void DrawModelExecute(void* ctx, void *state, const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld = NULL);
#else
	virtual int		DrawModel(int flags,
		IClientRenderable *pRenderable,
		ModelInstanceHandle_t instance,
		int entity_index,
		const model_t *model,
		Vector const& origin,
		QAngle const& angles,
		int skin,
		int body,
		int hitboxset,
		const matrix3x4_t *modelToWorld = NULL,
		const matrix3x4_t *pLightingOffset = NULL) = 0;

	// This causes a material to be used when rendering the model instead 
	// of the materials the model was compiled with
	virtual void	ForcedMaterialOverride(IMaterial *newMaterial, OverrideType_t nOverrideType = OVERRIDE_NORMAL) = 0;

	virtual void	SetViewTarget(const CStudioHdr *pStudioHdr, int nBodyIndex, const Vector& target) = 0;

	// Creates, destroys instance data to be associated with the model
	virtual ModelInstanceHandle_t CreateInstance(IClientRenderable *pRenderable, LightCacheHandle_t *pCache = NULL) = 0;
	virtual void DestroyInstance(ModelInstanceHandle_t handle) = 0;

	// Associates a particular lighting condition with a model instance handle.
	// FIXME: This feature currently only works for static props. To make it work for entities, etc.,
	// we must clean up the lightcache handles as the model instances are removed.
	// At the moment, since only the static prop manager uses this, it cleans up all LightCacheHandles 
	// at level shutdown.
	virtual void SetStaticLighting(ModelInstanceHandle_t handle, LightCacheHandle_t* pHandle) = 0;
	virtual LightCacheHandle_t GetStaticLighting(ModelInstanceHandle_t handle) = 0;

	// moves an existing InstanceHandle to a nex Renderable to keep decals etc. Models must be the same
	virtual bool ChangeInstance(ModelInstanceHandle_t handle, IClientRenderable *pRenderable) = 0;

	// Creates a decal on a model instance by doing a planar projection
	// along the ray. The material is the decal material, the radius is the
	// radius of the decal to create.
	virtual void AddDecal(ModelInstanceHandle_t handle, Ray_t const& ray,
		Vector const& decalUp, int decalIndex, int body, bool noPokeThru = false, int maxLODToDecal = ADDDECAL_TO_ALL_LODS) = 0;

	// Removes all the decals on a model instance
	virtual void RemoveAllDecals(ModelInstanceHandle_t handle) = 0;

	// Remove all decals from all models
	virtual void RemoveAllDecalsFromAllModels() = 0;

	// Shadow rendering, DrawModelShadowSetup returns the address of the bone-to-world array, NULL in case of error
	virtual matrix3x4_t* DrawModelShadowSetup(IClientRenderable *pRenderable, int body, int skin, DrawModelInfo_t *pInfo, matrix3x4_t *pCustomBoneToWorld = NULL) = 0;
	virtual void DrawModelShadow(IClientRenderable *pRenderable, const DrawModelInfo_t &info, matrix3x4_t *pCustomBoneToWorld = NULL) = 0;

	// This gets called when overbright, etc gets changed to recompute static prop lighting.
	virtual bool RecomputeStaticLighting(ModelInstanceHandle_t handle) = 0;

	virtual void ReleaseAllStaticPropColorData(void) = 0;
	virtual void RestoreAllStaticPropColorData(void) = 0;

	// Extended version of drawmodel
	virtual int	DrawModelEx(ModelRenderInfo_t &pInfo) = 0;

	virtual int	DrawModelExStaticProp(ModelRenderInfo_t &pInfo) = 0;

	virtual bool DrawModelSetup(ModelRenderInfo_t &pInfo, DrawModelState_t *pState, matrix3x4_t *pCustomBoneToWorld, matrix3x4_t** ppBoneToWorldOut) = 0;
	virtual void DrawModelExecute(const DrawModelState_t &state, const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld = NULL) = 0;

	// Sets up lighting context for a point in space
	virtual void SetupLighting(const Vector &vecCenter) = 0;

	// doesn't support any debug visualization modes or other model options, but draws static props in the
	// fastest way possible
	virtual int DrawStaticPropArrayFast(StaticPropRenderInfo_t *pProps, int count, bool bShadowDepth) = 0;

	// Allow client to override lighting state
	virtual void SuppressEngineLighting(bool bSuppress) = 0;

	virtual void SetupColorMeshes(int nTotalVerts) = 0;

	/*
	virtual void AddColoredDecal(ModelInstanceHandle_t handle, Ray_t const& ray,
		Vector const& decalUp, int decalIndex, int body, Color cColor, bool noPokeThru = false, int maxLODToDecal = ADDDECAL_TO_ALL_LODS) = 0;
	*/

	virtual void SetupLightingEx(Vector const&, unsigned short) = 0;

	virtual void GetBrightestShadowingLightSource(Vector const&, Vector&, Vector&, bool) = 0;

	/*
	virtual void ComputeLightingState(int, LightingQuery_t const*, MaterialLightingState_t*, ITexture**) = 0;

	virtual void GetModelDecalHandles(StudioDecalHandle_t__**, int, int, unsigned short const*) = 0;

	virtual void ComputeStaticLightingState(int, StaticLightingQuery_t const*, MaterialLightingState_t*, MaterialLightingState_t*, ColorMeshInfo_t**, ITexture**, memhandle_t__**) = 0;

	virtual void CleanupStaticLightingState(int, memhandle_t__**) = 0;

	virtual void GetItemName(unsigned int, void const*, char*, unsigned int) = 0;
	*/
#endif
};