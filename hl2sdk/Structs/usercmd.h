#pragma once
#include "../Utils/checksum_crc.h"
#include "../../l4d2Simple2/vector.h"
#include <Windows.h>
#include <cstdint>

class CUserCmd
{
public:
	CRC32_t GetChecksum() const;

	LPVOID __vtable;			// 0x00 - VTable
	int32_t command_number;		// 0x04	For matching server and client commands for debugging
	int32_t tick_count;			// 0x08	the tick the client created this command
	QAngle viewangles;			// 0x0C	Player instantaneous view angles.
	// Vector aimdirection;		// l4d2 没有这个东西
	float forwardmove;			// 0x18
	float sidemove;				// 0x1C
	float upmove;				// 0x20
	int32_t buttons;			// 0x24	Attack button states
	byte impulse;				// 0x28
	int32_t weaponselect;		// 0x2A	Current weapon id
	int32_t weaponsubtype;		// 0x2E
	int32_t random_seed;		// 0x34	For shared random functions
	int16_t mousedx;			// 0x38	mouse accum in x from create move
	int16_t mousedy;			// 0x3A	mouse accum in y from create move

	// 这几个不参与CRC验证的
	bool hasbeenpredicted;		// 0x3C
	Vector headangles;			// 0x40
	Vector headoffset;			// 0x4C
};	// Size = 0x4C + 0xC = 0x58

class CVerifiedUserCmd
{
public:
	CUserCmd	m_cmd;
	CRC32_t		m_crc;
};
