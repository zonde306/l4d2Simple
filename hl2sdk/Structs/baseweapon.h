#pragma once

#include "baseentity.h"

#define MAX_SHOOT_SOUNDS 16
#define MAX_WEAPON_STRING 80
#define MAX_WEAPON_PREFIX 16
#define MAX_WEAPON_AMMO_NAME 32

typedef enum 
{
	EMPTY,
	SINGLE,
	SINGLE_NPC,
	WPN_DOUBLE,
	DOUBLE_NPC,
	BURST,
	RELOAD,
	RELOAD_NPC,
	MELEE_MISS,
	MELEE_HIT,
	MELEE_HIT_WORLD,
	SPECIAL1,
	SPECIAL2,
	SPECIAL3,
	TAUNT,
	DEPLOY,
	NUM_SHOOT_SOUND_TYPES
} WeaponSound_t;

struct FileWeaponInfo_t
{
private:
	byte pad_0x0000[0x4];

public:
	bool bParsedScript;
	bool bLoadedHudElements;
	char szClassName[MAX_WEAPON_STRING];
	char szPrintName[MAX_WEAPON_STRING];
	char szViewModel[MAX_WEAPON_STRING];
	char szWorldModel[MAX_WEAPON_STRING];
	char szAnimationPrefix[MAX_WEAPON_PREFIX];
	std::int16_t pad1;
	std::int32_t iSlot;
	std::int32_t iPosition;
	std::int32_t iMaxClip1;
	std::int32_t iMaxClip2;
	std::int32_t iDefaultClip1;
	std::int32_t iDefaultClip12;
	std::int32_t iWeight;
	std::int32_t iRumble;
	bool bAutoSwitchTo;
	bool bAutoSwitchFrom;
	std::int32_t iFlags;
	char szAmmo1[MAX_WEAPON_AMMO_NAME];
	char szAmmo2[MAX_WEAPON_AMMO_NAME];
	char aShootSounds[NUM_SHOOT_SOUND_TYPES][MAX_WEAPON_STRING];
	int iAmmoType;
	int iAmmo2Type;
	bool m_bMeleeWeapon;
	bool m_bBuiltRightHanded;
	bool m_bAllowFlipping;
	int iSpriteCount;
};

class CBasePlayer;

class CBaseWeapon : public CBaseEntity
{
public:
	float& GetSpread();
	FileWeaponInfo_t* GetWeaponInfo();
	FileWeaponInfo_t* GetWeaponData();
	const char* GetWeaponName();
	int GetWeaponID();
	int GetWeaponId();
	void UpdateSpread();

	CBasePlayer* GetOwner();
	int GetClip();

	bool IsReloading();

	int GetAmmoType();
	int GetAmmo();

	float GetNextPrimaryAttack();
	float GetPrimaryAttackDelay();
	float GetNextSecondryAttack();
	float GetSecondryAttackDelay();

	bool CanFire();
	bool IsFireGun();
	bool CanShove();

	float GetMeleeDelay();
};