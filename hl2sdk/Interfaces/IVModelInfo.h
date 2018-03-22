#pragma once
#include "../definitions.h"
#include "../Structs/matrix.h"
#include "../Structs/trace.h"
#include "../../l4d2Simple2/vector.h"

enum Bone: int
{
	BONE_PELVIS = 0,
	LEAN_ROOT,
	CAM_DRIVER,
	BONE_HIP,
	BONE_LOWER_SPINAL_COLUMN,
	BONE_MIDDLE_SPINAL_COLUMN,
	BONE_UPPER_SPINAL_COLUMN,
	BONE_NECK,
	BONE_HEAD,
};

enum Hitboxes: int
{
	HITBOX_HEAD = 0,
	HITBOX_NECK,
	HITBOX_PELVIS,
	HITBOX_SPINE,
	HITBOX_LEGS,
	HITBOX_ARMS,
};

struct mstudiobbox_t
{
	int					bone;
	int					group;				// intersection group
	Vector				bbmin;				// bounding box
	Vector				bbmax;
	int					szhitboxnameindex;	// offset to the name of the hitbox.
	// int				unused[8];
	int					m_iPad01[3];
	float				m_flRadius;
	int					m_iPad02[4];

	const char* pszHitboxName()
	{
		if (szhitboxnameindex == 0)
			return "";

		return ((char*)this) + szhitboxnameindex;
	}

	mstudiobbox_t() {}

private:
	mstudiobbox_t(const mstudiobbox_t& vOther);
};

struct model_t
{
	DWORD		unknown;
	char        name[255];
};

//-----------------------------------------------------------------------------
// Forward declarations
//-----------------------------------------------------------------------------
class IMaterial;
struct vcollide_t;
class Vector;
class QAngle;
class CGameTrace;
struct cplane_t;
struct studiohdr_t;
struct virtualmodel_t;
typedef unsigned char byte;
struct virtualterrainparams_t;
class CPhysCollide;
typedef unsigned short MDLHandle_t;
class CUtlBuffer;
class IClientRenderable;
class Quaternion;
struct mstudioanimdesc_t;
struct mstudioseqdesc_t;
struct mstudiobodyparts_t;
struct mstudiotexture_t;

class RadianEuler
{
public:
	inline RadianEuler( void )							{ }
	inline RadianEuler( float X, float Y, float Z )		{ x = X; y = Y; z = Z; }
	inline RadianEuler( Quaternion const &q );	// evil auto type promotion!!!
	inline RadianEuler( QAngle const &angles );	// evil auto type promotion!!!

												// Initialization
	inline void Init( float ix = 0.0f, float iy = 0.0f, float iz = 0.0f )	{ x = ix; y = iy; z = iz; }

	//	conversion to qangle
	QAngle ToQAngle( void ) const;
	bool IsValid() const;
	void Invalidate();

	inline float *Base() { return &x; }
	inline const float *Base() const { return &x; }

	// array access...
	float operator[]( int i ) const;
	float& operator[]( int i );

	float x, y, z;
};

class Quaternion				// same data-layout as engine's vec4_t,
{								//		which is a float[4]
public:
	inline Quaternion( void )	{}
	inline Quaternion( float ix, float iy, float iz, float iw ) : x( ix ), y( iy ), z( iz ), w( iw ) { }
	inline Quaternion( RadianEuler const &angle );	// evil auto type promotion!!!

	inline void Init( float ix = 0.0f, float iy = 0.0f, float iz = 0.0f, float iw = 0.0f )	{ x = ix; y = iy; z = iz; w = iw; }

	bool IsValid() const;
	void Invalidate();

	bool operator==( const Quaternion &src ) const;
	bool operator!=( const Quaternion &src ) const;

	float* Base() { return ( float* )this; }
	const float* Base() const { return ( float* )this; }

	// array access...
	float operator[]( int i ) const;
	inline float& operator[](int i)
	{
		switch (i)
		{
		case 0:
			return x;
		case 1:
			return y;
		case 2:
			return z;
		case 3:
			return w;
		}

		return z;
	};

	float x, y, z, w;
};

// SetupBones 用的 mask
#define BONE_CALCULATE_MASK				0x1F
#define BONE_PHYSICALLY_SIMULATED		0x01	// bone is physically simulated when physics are active
#define BONE_PHYSICS_PROCEDURAL			0x02	// procedural when physics is active
#define BONE_ALWAYS_PROCEDURAL			0x04	// bone is always procedurally animated
#define BONE_SCREEN_ALIGN_SPHERE		0x08	// bone aligns to the screen, not constrained in motion.
#define BONE_SCREEN_ALIGN_CYLINDER		0x10	// bone aligns to the screen, constrained by it's own axis.
#define BONE_USED_MASK					0x0007FF00
#define BONE_USED_BY_ANYTHING			0x0007FF00
#define BONE_USED_BY_HITBOX				0x00000100	// bone (or child) is used by a hit box
#define BONE_USED_BY_ATTACHMENT			0x00000200	// bone (or child) is used by an attachment point
#define BONE_USED_BY_VERTEX_MASK		0x0003FC00
#define BONE_USED_BY_VERTEX_LOD0		0x00000400	// bone (or child) is used by the toplevel model via skinned vertex
#define BONE_USED_BY_VERTEX_LOD1		0x00000800	
#define BONE_USED_BY_VERTEX_LOD2		0x00001000  
#define BONE_USED_BY_VERTEX_LOD3		0x00002000
#define BONE_USED_BY_VERTEX_LOD4		0x00004000
#define BONE_USED_BY_VERTEX_LOD5		0x00008000
#define BONE_USED_BY_VERTEX_LOD6		0x00010000
#define BONE_USED_BY_VERTEX_LOD7		0x00020000
#define BONE_USED_BY_BONE_MERGE			0x00040000	// bone is available for bone merge to occur against it

// DrawModel 用的 flags
#define STUDIO_NONE						0x00000000
#define STUDIO_RENDER					0x00000001
#define STUDIO_VIEWXFORMATTACHMENTS		0x00000002
#define STUDIO_DRAWTRANSLUCENTSUBMODELS 0x00000004
#define STUDIO_TWOPASS					0x00000008
#define STUDIO_STATIC_LIGHTING			0x00000010
#define STUDIO_WIREFRAME				0x00000020
#define STUDIO_ITEM_BLINK				0x00000040
#define STUDIO_NOSHADOWS				0x00000080
#define STUDIO_WIREFRAME_VCOLLIDE		0x00000100
#define STUDIO_GENERATE_STATS			0x01000000	// Not a studio flag, but used to flag when we want studio stats
#define STUDIO_SSAODEPTHTEXTURE			0x08000000	// Not a studio flag, but used to flag model as using shadow depth material override
#define STUDIO_SHADOWDEPTHTEXTURE		0x40000000	// Not a studio flag, but used to flag model as using shadow depth material override
#define STUDIO_TRANSPARENCY				0x80000000	// Not a studio flag, but used to flag model as a non-sorting brush model

struct mstudiohitboxset_t
{
	int sznameindex;
	int numhitboxes;
	int hitboxindex;

	inline char * const pszName( void ) const
	{
		return ( ( char* )this ) + sznameindex;
	}

	inline mstudiobbox_t* GetHitbox( int i ) const
	{
		return ( mstudiobbox_t* )( ( ( byte* )this ) + hitboxindex ) + i;
	}
};

struct mstudiobone_t
{
	int					sznameindex;
	int		 			parent;		// parent bone
	int					bonecontroller[6];	// bone controller index, -1 == none

	// default values
	Vector				pos;
	Quaternion			quat;
	Vector				rot;
	// compression scale
	Vector				posscale;
	Vector				rotscale;

	VMatrix				poseToBone;
	Quaternion			qAlignment;
	int					flags;
	int					proctype;
	int					procindex;		// procedural rule
	mutable int			physicsbone;	// index into physically simulated bone
	int					surfacepropidx;	// index into string tablefor property name
	int					contents;		// See BSPFlags.h for the contents flags

	int					unused[8];		// remove as appropriate

	inline char * const pszName(void) const { return ((char *)this) + sznameindex; }
	inline void *pProcedure() const { if (procindex == 0) return NULL; else return  (void *)(((byte *)this) + procindex); };
	inline char * const pszSurfaceProp(void) const { return ((char *)this) + surfacepropidx; }

	mstudiobone_t() {}
private:
	// No copy constructors allowed
	mstudiobone_t(const mstudiobone_t& vOther);
};

struct mstudiobonecontroller_t
{
	int					bone;	// -1 == 0
	int					type;	// X, Y, Z, XR, YR, ZR, M
	float				start;
	float				end;
	int					rest;	// byte index value at rest
	int					inputfield;	// 0-3 user set controller, 4 mouth
	int					unused[ 8 ];
};

struct studiohdr_t
{
	int					id;
	int					version;
	int					checksum;		// this has to be the same in the phy and vtx files to load!
	char				name[64];
	int					length;

	Vector				eyeposition;	// ideal eye position
	Vector				illumposition;	// illumination center
	Vector				hull_min;		// ideal movement hull size
	Vector				hull_max;
	Vector				view_bbmin;		// clipping bounding box
	Vector				view_bbmax;

	int					flags;
	int					numbones;			// bones
	int					boneindex;

	int					numbonecontrollers;		// bone controllers
	int					bonecontrollerindex;

	int					numhitboxsets;
	int					hitboxsetindex;

	int numlocalanim;
	int localanimindex;
	int numlocalseq;
	int localseqindex;
	mutable int activitylistversion;
	mutable int eventsindexed;
	int numtextures;
	int textureindex;
	int numcdtextures;
	int cdtextureindex;
	int numskinref;
	int numskinfamilies;
	int skinindex;
	int numbodyparts;
	int bodypartindex;
	int numlocalattachments;
	int localattachmentindex;
	int numlocalnodes;
	int localnodeindex;
	int localnodenameindex;
	int numflexdesc;
	int flexdescindex;
	int numflexcontrollers;
	int flexcontrollerindex;
	int numflexrules;
	int flexruleindex;
	int numikchains;
	int ikchainindex;
	int nummouths;
	int mouthindex;
	int numlocalposeparameters;
	int localposeparamindex;
	int surfacepropindex;
	int keyvalueindex;
	int keyvaluesize;
	int numlocalikautoplaylocks;
	int localikautoplaylockindex;
	float mass;
	int contents;
	int numincludemodels;
	int includemodelindex;
	mutable void *virtualModel;
	int szanimblocknameindex;
	int numanimblocks;
	int animblockindex;
	mutable void *animblockModel;
	int bonetablebynameindex;
	void *pVertexBase;
	void *pIndexBase;
	BYTE constdirectionallightdot;
	BYTE rootLOD;
	BYTE numAllowedRootLODs;
	BYTE unused[1];
	int unused4;
	int numflexcontrollerui;
	int flexcontrolleruiindex;
	float flVertAnimFixedPointScale;
	int unused3[1];
	int studiohdr2index;
	int unused2[1];

	int					RemapSeqBone(int iSequence, int iLocalBone) const;	// maps local sequence bone to global bone
	int					RemapAnimBone(int iAnim, int iLocalBone) const;		// maps local animations bone to global bone
	inline const char *	pszName(void) const { return name; }

	inline mstudiobone_t *GetBone(int i) const { return (mstudiobone_t *)(((byte *)this) + boneindex) + i; };
	inline void *pBonecontroller(int i) const { return  (((byte *)this) + bonecontrollerindex) + i; };

	// Look up hitbox set by index
	mstudiohitboxset_t	*GetHitboxSet(int i) const
	{
		return (mstudiohitboxset_t *)(((byte *)this) + hitboxsetindex) + i;
	};

	// Calls through to hitbox to determine size of specified set
	inline mstudiobbox_t *GetHitbox(int i, int set) const
	{
		mstudiohitboxset_t const *s = GetHitboxSet(set);
		if (!s)
			return NULL;

		return s->GetHitbox(i);
	};

	// Calls through to set to get hitbox count for set
	inline int			iHitboxCount(int set) const
	{
		mstudiohitboxset_t const *s = GetHitboxSet(set);
		if (!s)
			return 0;

		return s->numhitboxes;
	};


	studiohdr_t() {}

private:
	// No copy constructors allowed
	studiohdr_t(const studiohdr_t& vOther);

	friend struct virtualmodel_t;
};


//-----------------------------------------------------------------------------
// Indicates the type of translucency of an unmodulated renderable
//-----------------------------------------------------------------------------
enum RenderableTranslucencyType_t
{
	RENDERABLE_IS_OPAQUE = 0,
	RENDERABLE_IS_TRANSLUCENT,
	RENDERABLE_IS_TWO_PASS,	// has both translucent and opaque sub-partsa
};

class IModelLoadCallback
{
public:
	virtual void OnModelLoadComplete(const model_t* pModel) = 0;

protected:
	// Protected destructor so that nobody tries to delete via this interface.
	// Automatically unregisters if the callback is destroyed while still pending.
	~IModelLoadCallback();
};

//-----------------------------------------------------------------------------
// Model info interface
//-----------------------------------------------------------------------------

class IVModelInfo
{
public:
#ifdef _CSGO
	virtual							~IVModelInfo( void ) { }
	virtual const model_t			*GetModel( int modelindex ) const = 0;
	// Returns index of model by name
	virtual int						GetModelIndex( const char *name ) const = 0;
	// Returns name of model
	virtual const char				*GetModelName( const model_t *model ) const = 0;
	virtual vcollide_t				*GetVCollide( const model_t *model ) const = 0;
	virtual vcollide_t				*GetVCollide( int modelindex ) const = 0;
	virtual void					GetModelBounds( const model_t *model, Vector& mins, Vector& maxs ) const = 0;
	virtual	void					GetModelRenderBounds( const model_t *model, Vector& mins, Vector& maxs ) const = 0;
	virtual int						GetModelFrameCount( const model_t *model ) const = 0;
	virtual int						GetModelType( const model_t *model ) const = 0;
	virtual void					*GetModelExtraData( const model_t *model ) = 0;
	virtual bool					ModelHasMaterialProxy( const model_t *model ) const = 0;
	virtual bool					IsTranslucent( model_t const* model ) const = 0;
	virtual bool					IsTranslucentTwoPass( const model_t *model ) const = 0;
	virtual void					Unused0() {};
	virtual RenderableTranslucencyType_t ComputeTranslucencyType( const model_t *model, int nSkin, int nBody ) = 0;
	virtual int						GetModelMaterialCount( const model_t* model ) const = 0;
	virtual void					GetModelMaterials( const model_t *model, int count, IMaterial** ppMaterials ) = 0;
	virtual bool					IsModelVertexLit( const model_t *model ) const = 0;
	virtual const char				*GetModelKeyValueText( const model_t *model ) = 0;
	virtual bool					GetModelKeyValue( const model_t *model, CUtlBuffer &buf ) = 0; // supports keyvalue blocks in submodels
	virtual float					GetModelRadius( const model_t *model ) = 0;

	virtual const studiohdr_t		*FindModel( const studiohdr_t *pStudioHdr, void **cache, const char *modelname ) const = 0;
	virtual const studiohdr_t		*FindModel( void *cache ) const = 0;
	virtual	virtualmodel_t			*GetVirtualModel( const studiohdr_t *pStudioHdr ) const = 0;
	virtual byte					*GetAnimBlock( const studiohdr_t *pStudioHdr, int iBlock ) const = 0;
	virtual bool					HasAnimBlockBeenPreloaded( studiohdr_t const*, int ) const = 0;

	// Available on client only!!!
	virtual void					GetModelMaterialColorAndLighting( const model_t *model, Vector const& origin,
		QAngle const& angles, trace_t* pTrace,
		Vector& lighting, Vector& matColor ) = 0;
	virtual void					GetIlluminationPoint( const model_t *model, IClientRenderable *pRenderable, Vector const& origin,
		QAngle const& angles, Vector* pLightingCenter ) = 0;

	virtual int						GetModelContents( int modelIndex ) const = 0;
	virtual studiohdr_t				*GetStudioModel( const model_t *mod ) = 0;
	virtual int						GetModelSpriteWidth( const model_t *model ) const = 0;
	virtual int						GetModelSpriteHeight( const model_t *model ) const = 0;

	// Sets/gets a map-specified fade range (client only)
	virtual void					SetLevelScreenFadeRange( float flMinSize, float flMaxSize ) = 0;
	virtual void					GetLevelScreenFadeRange( float *pMinArea, float *pMaxArea ) const = 0;

	// Sets/gets a map-specified per-view fade range (client only)
	virtual void					SetViewScreenFadeRange( float flMinSize, float flMaxSize ) = 0;

	// Computes fade alpha based on distance fade + screen fade (client only)
	virtual unsigned char			ComputeLevelScreenFade( const Vector &vecAbsOrigin, float flRadius, float flFadeScale ) const = 0;
	virtual unsigned char			ComputeViewScreenFade( const Vector &vecAbsOrigin, float flRadius, float flFadeScale ) const = 0;

	// both client and server
	virtual int						GetAutoplayList( const studiohdr_t *pStudioHdr, unsigned short **pAutoplayList ) const = 0;

	// Gets a virtual terrain collision model (creates if necessary)
	// NOTE: This may return NULL if the terrain model cannot be virtualized
	virtual CPhysCollide			*GetCollideForVirtualTerrain( int index ) = 0;
	virtual bool					IsUsingFBTexture( const model_t *model, int nSkin, int nBody, void /*IClientRenderable*/ *pClientRenderable ) const = 0;
	virtual const model_t			*FindOrLoadModel( const char *name ) const = 0;
	virtual MDLHandle_t				GetCacheHandle( const model_t *model ) const = 0;
	// Returns planes of non-nodraw brush model surfaces
	virtual int						GetBrushModelPlaneCount( const model_t *model ) const = 0;
	virtual void					GetBrushModelPlane( const model_t *model, int nIndex, cplane_t &plane, Vector *pOrigin ) const = 0;
	virtual int						GetSurfacepropsForVirtualTerrain( int index ) = 0;
	virtual bool					UsesEnvCubemap( const model_t *model ) const = 0;
	virtual bool					UsesStaticLighting( const model_t *model ) const = 0;
#else
	virtual							~IVModelInfo(void) { }

	// Returns model_t* pointer for a model given a precached or dynamic model index.
	virtual const model_t			*GetModel(int modelindex) = 0;

	// Returns index of model by name for precached or known dynamic models.
	// Does not adjust reference count for dynamic models.
	virtual int						GetModelIndex(const char *name) const = 0;

	// Returns name of model
	virtual const char				*GetModelName(const model_t *model) const = 0;
	virtual vcollide_t				*GetVCollide(const model_t *model) = 0;
	virtual vcollide_t				*GetVCollide(int modelindex) = 0;
	virtual void					GetModelBounds(const model_t *model, Vector& mins, Vector& maxs) const = 0;
	virtual	void					GetModelRenderBounds(const model_t *model, Vector& mins, Vector& maxs) const = 0;
	virtual int						GetModelFrameCount(const model_t *model) const = 0;
	virtual int						GetModelType(const model_t *model) const = 0;
	virtual void					*GetModelExtraData(const model_t *model) = 0;
	virtual bool					ModelHasMaterialProxy(const model_t *model) const = 0;
	virtual bool					IsTranslucent(model_t const* model) const = 0;
	virtual bool					IsTranslucentTwoPass(const model_t *model) const = 0;
	virtual void					RecomputeTranslucency(const model_t *model, int nSkin, int nBody, void /*IClientRenderable*/ *pClientRenderable, float fInstanceAlphaModulate = 1.0f) = 0;
	virtual int						GetModelMaterialCount(const model_t* model) const = 0;
	virtual void					GetModelMaterials(const model_t *model, int count, IMaterial** ppMaterial) = 0;
	virtual bool					IsModelVertexLit(const model_t *model) const = 0;
	virtual const char				*GetModelKeyValueText(const model_t *model) = 0;
	virtual bool					GetModelKeyValue(const model_t *model, CUtlBuffer &buf) = 0; // supports keyvalue blocks in submodels
	virtual float					GetModelRadius(const model_t *model) = 0;

	virtual const studiohdr_t		*FindModel(const studiohdr_t *pStudioHdr, void **cache, const char *modelname) const = 0;
	virtual const studiohdr_t		*FindModel(void *cache) const = 0;
	virtual	virtualmodel_t			*GetVirtualModel(const studiohdr_t *pStudioHdr) const = 0;
	virtual byte					*GetAnimBlock(const studiohdr_t *pStudioHdr, int iBlock) const = 0;

	// Available on client only!!!
	virtual void					GetModelMaterialColorAndLighting(const model_t *model, Vector const& origin,
		QAngle const& angles, trace_t* pTrace,
		Vector& lighting, Vector& matColor) = 0;
	virtual void					GetIlluminationPoint(const model_t *model, IClientRenderable *pRenderable, Vector const& origin,
		QAngle const& angles, Vector* pLightingCenter) = 0;

	virtual int						GetModelContents(int modelIndex) = 0;
	virtual studiohdr_t				*GetStudiomodel(const model_t *mod) = 0;
	virtual int						GetModelSpriteWidth(const model_t *model) const = 0;
	virtual int						GetModelSpriteHeight(const model_t *model) const = 0;

	// Sets/gets a map-specified fade range (client only)
	virtual void					SetLevelScreenFadeRange(float flMinSize, float flMaxSize) = 0;
	virtual void					GetLevelScreenFadeRange(float *pMinArea, float *pMaxArea) const = 0;

	// Sets/gets a map-specified per-view fade range (client only)
	virtual void					SetViewScreenFadeRange(float flMinSize, float flMaxSize) = 0;

	// Computes fade alpha based on distance fade + screen fade (client only)
	virtual unsigned char			ComputeLevelScreenFade(const Vector &vecAbsOrigin, float flRadius, float flFadeScale) const = 0;
	virtual unsigned char			ComputeViewScreenFade(const Vector &vecAbsOrigin, float flRadius, float flFadeScale) const = 0;

	// both client and server
	virtual int						GetAutoplayList(const studiohdr_t *pStudioHdr, unsigned short **pAutoplayList) const = 0;

	// Gets a virtual terrain collision model (creates if necessary)
	// NOTE: This may return NULL if the terrain model cannot be virtualized
	virtual CPhysCollide			*GetCollideForVirtualTerrain(int index) = 0;

	virtual bool					IsUsingFBTexture(const model_t *model, int nSkin, int nBody, void /*IClientRenderable*/ *pClientRenderable) const = 0;

	// Obsolete methods. These are left in to maintain binary compatibility with clients using the IVModelInfo old version.
	virtual const model_t			*FindOrLoadModel(const char *name) { /*Warning("IVModelInfo::FindOrLoadModel is now obsolte.\n");*/ return NULL; }
	virtual void					InitDynamicModels() { /*Warning("IVModelInfo::InitDynamicModels is now obsolte.\n");*/ }
	virtual void					ShutdownDynamicModels() { /*Warning("IVModelInfo::ShutdownDynamicModels is now obsolte.\n");*/ }
	virtual void					AddDynamicModel(const char *name, int nModelIndex = -1) { /*Warning("IVModelInfo::AddDynamicModel is now obsolte.\n");*/ }
	virtual void					ReferenceModel(int modelindex) { /*Warning("IVModelInfo::ReferenceModel is now obsolte.\n");*/ }
	virtual void					UnreferenceModel(int modelindex) { /*Warning("IVModelInfo::UnreferenceModel is now obsolte.\n");*/ }
	virtual void					CleanupDynamicModels(bool bForce = false) { /*Warning("IVModelInfo::CleanupDynamicModels is now obsolte.\n");*/ }

	virtual MDLHandle_t				GetCacheHandle(const model_t *model) const = 0;

	// Returns planes of non-nodraw brush model surfaces
	virtual int						GetBrushModelPlaneCount(const model_t *model) const = 0;
	virtual void					GetBrushModelPlane(const model_t *model, int nIndex, cplane_t &plane, Vector *pOrigin) const = 0;
	virtual int						GetSurfacepropsForVirtualTerrain(int index) = 0;

	// Poked by engine host system
	virtual void					OnLevelChange() = 0;

	virtual int						GetModelClientSideIndex(const char *name) const = 0;

	// Returns index of model by name, dynamically registered if not already known.
	virtual int						RegisterDynamicModel(const char *name, bool bClientSide) = 0;

	virtual bool					IsDynamicModelLoading(int modelIndex) = 0;

	virtual void					AddRefDynamicModel(int modelIndex) = 0;
	virtual void					ReleaseDynamicModel(int modelIndex) = 0;

	// Registers callback for when dynamic model has finished loading.
	// Automatically adds reference, pair with ReleaseDynamicModel.
	virtual bool					RegisterModelLoadCallback(int modelindex, IModelLoadCallback* pCallback, bool bCallImmediatelyIfLoaded = true) = 0;
	virtual void					UnregisterModelLoadCallback(int modelindex, IModelLoadCallback* pCallback) = 0;

	virtual void OnDynamicModelsStringTableChange(int nStringIndex, const char *pString, const void *pData) = 0;
#endif
};


class IVModelInfoClient : public IVModelInfo
{
public:
};


struct virtualterrainparams_t
{
	// UNDONE: Add grouping here, specified in BSP file? (test grouping to see if this is necessary)
	int index;
};
