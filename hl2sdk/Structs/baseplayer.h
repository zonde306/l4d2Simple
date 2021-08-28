#pragma once

#include "baseentity.h"
#include "playerinfo.h"

class CBaseWeapon;

enum ETeam
{
	TEAM_UNKNOWN = 0,
	TEAM_SPECTATOR = 1,
	TEAM_SURVIVORS = 2,
	TEAM_INFECTED = 3,
	TEAM_L4D1_SURVIVORS = 4
};

class CBasePlayer : public CBaseEntity
{
public:
	Vector GetEyePosition();
	QAngle GetEyeAngles();
	float GetFriction();
	int GetTeam();
	Vector GetVelocity();
	int GetHealth();
	bool IsAlive();
	Vector& GetPunch();
	int& GetTickBase();
	int& GetFlags();
	CBaseEntity* GetGroundEntity();
	int GetWaterLevel();
	std::string GetName();
	bool CanShove();
	bool CanAttack();
	bool CanBeShove();
	bool IsReadyToShove();

	std::pair<Vector, Vector> GetBoundingBox();

	bool IsIncapacitated();
	bool IsHangingFromLedge();

	int GetTempHealth();
	CBaseWeapon* GetActiveWeapon();
	int GetCrosshairID();

	bool IsGhost();

	int GetAmmo(int ammoType);

	CBasePlayer* GetCurrentAttacker();
	CBasePlayer* GetCurrentVictim();

	bool IsOnGround();
	Vector GetHeadOrigin();
	bool IsDying();

	std::string GetCharacterName();
	ZombieClass_t GetZombieType();

	bool IsSurvivor();
	bool IsSpecialInfected();
	bool IsCommonInfected();
	bool IsWitch();

	Vector GetChestOrigin();
	bool IsLunging();
	CBasePlayer* GetCurrentTongueTarget();

	bool IsStaggering();
	bool IsGettingUp();
};