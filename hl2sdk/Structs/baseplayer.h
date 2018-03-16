#pragma once
#include "baseentity.h"

class CBasePlayer : public CBaseEntity
{
public:
	Vector GetEyePosition();
	QAngle GetEyeAngles();
	float GetFriction();
	int GetTeam();
	Vector GetVelocity();
	int GetHealth();
	int GetTempHealth();
	bool IsAlive();
	MoveType_t GetMoveType();
	Vector& GetPunch();
};

