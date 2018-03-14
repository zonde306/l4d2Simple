#pragma once
#include "../Structs/baseentity.h"

class IMoveHelper
{
private:
	virtual void UnknownVirtual() = 0;
public:
	virtual void SetHost(CBaseEntity* host) = 0;
};