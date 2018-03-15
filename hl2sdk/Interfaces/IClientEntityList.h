#pragma once
#include "../Structs/baseentity.h"
#include "../Structs/handle.h"
#include "IClientNetworkable.h"
#include "IClientUnknown.h"
#include "IClientEntity.h"

class IClientEntityList
{
public:
	// Get IClientNetworkable interface for specified entity
	virtual IClientNetworkable*	GetClientNetworkable(int entnum) = 0;
	virtual IClientNetworkable*	GetClientNetworkableFromHandle(CBaseHandle hEnt) = 0;
	virtual IClientUnknown*		GetClientUnknownFromHandle(CBaseHandle hEnt) = 0;

	// NOTE: This function is only a convenience wrapper.
	// It returns GetClientNetworkable( entnum )->GetIClientEntity().
	virtual IClientEntity*		GetClientEntity(int entnum) = 0;
	virtual IClientEntity*		GetClientEntityFromHandle(CBaseHandle hEnt) = 0;

	// Returns number of entities currently in use
	virtual int					NumberOfEntities(bool bIncludeNonNetworkable) = 0;

	virtual void				Unknown_6() = 0;
	virtual void				Unknown_7() = 0;

	// Returns highest index actually used
	virtual int					GetHighestEntityIndex(void) = 0;

	// Sizes entity list to specified size
	virtual void				SetMaxEntities(int maxents) = 0;
	virtual int					GetMaxEntities() = 0;
};