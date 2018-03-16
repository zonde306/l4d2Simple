#pragma once
#include "baseentity.h"

struct FileWeaponInfo_t
{
private:
	byte pad_0x0000[0x4];		// 0x0000 - VTable

public:
	bool bParsedScript;
	bool bLoadedHudElements;
	char szClassName[80];
	char szPrintName[80];
	char szViewModel[80];
	char szWorldModel[80];
	char szAnimationPrefix[16];
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
};

class CBaseWeapon : public CBaseEntity
{
public:
	float& GetSpread();
	FileWeaponInfo_t* GetWeaponData();
	const char* GetWeaponName();
	int GetWeaponId();
};
