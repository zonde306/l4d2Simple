#pragma once
#include "../Utils/checksum_crc.h"
#include "../../l4d2Simple2/vector.h"
#include <Windows.h>
#include <cstdint>

class CUserCmd
{
public:
	CRC32_t GetChecksum() const;

	byte pad_0x0000[0x4];		// 0x0000 - VTable
	int32_t command_number;		// 0x0004	For matching server and client commands for debugging
	int32_t tick_count;			// 0x0008	the tick the client created this command
	QAngle viewangles;			// 0x000C	Player instantaneous view angles.
	float forwardmove;			// 0x0018
	float sidemove;				// 0x001C
	float upmove;				// 0x0020
	int32_t buttons;			// 0x0024	Attack button states
	byte impulse;				// 0x0028
	int32_t weaponselect;		// 0x002A	Current weapon id
	int32_t weaponsubtype;		// 0x002E
	// byte pad_0x0028[0xC];	// 0x0032
	int32_t random_seed;		// 0x0034	For shared random functions
	int16_t mousedx;			// 0x0038	mouse accum in x from create move
	int16_t mousedy;			// 0x003A	mouse accum in y from create move
	// Vector headangles;		// 0x003C
	// Vector headoffset;		// 0x0048
	byte pad_0x0038[0x1C];		// 0x0038
};	// Size = 0x0058

class CVerifiedUserCmd
{
public:
	CUserCmd	m_cmd;
	CRC32_t		m_crc;
};

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
