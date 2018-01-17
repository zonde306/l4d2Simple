#pragma once
#include "../Structs/matrix.h"
#include "../Structs/baseentity.h"

class model_t;

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

class IVModelRender
{
public:
	void ForcedMaterialOverride( IMaterial *mat );
	void DrawModelExecute( void* ctx, void *state, const ModelRenderInfo_t &pInfo, matrix3x4_t *pCustomBoneToWorld = NULL );
};