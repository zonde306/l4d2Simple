#pragma once
#include "IClientNetworkable.h"
#include "IClientThinkable.h"
#include "IClientEntity.h"
#include "IClientRenderable.h"
#include "ICollideable.h"
#include "../Structs/baseentity.h"
#include "../Structs/handle.h"

class IClientUnknown : public IHandleEntity
{
public:
	virtual ICollideable*		GetCollideable() = 0;
	virtual IClientNetworkable*	GetClientNetworkable() = 0;
	virtual IClientRenderable*	GetClientRenderable() = 0;
	virtual IClientEntity*		GetIClientEntity() = 0;
	virtual CBaseEntity*		GetBaseEntity() = 0;
	virtual IClientThinkable*	GetClientThinkable() = 0;
};
