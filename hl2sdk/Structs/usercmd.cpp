#include "usercmd.h"
#include "../../l4d2Simple2/xorstr.h"

#define SIG_CHECKSUM	XorStr("55 8B EC 51 56 8D 45 FC 50 8B F1 E8 ? ? ? ? 6A 04 8D 4E 04 51 8D 55 FC 52 E8")

CRC32_t CUserCmd::GetChecksum() const
{
	CRC32_t crc;

	/*
		CRC32_Init(&crc);
		sub_103C16E0(&crc, v1 + 4, 4);
		sub_103C16E0(&crc, v1 + 8, 4);
		sub_103C16E0(&crc, v1 + 12, 12);
		sub_103C16E0(&crc, v1 + 24, 4);
		sub_103C16E0(&crc, v1 + 28, 4);
		sub_103C16E0(&crc, v1 + 32, 4);
		sub_103C16E0(&crc, v1 + 36, 4);
		sub_103C16E0(&crc, v1 + 40, 1);
		sub_103C16E0(&crc, v1 + 44, 4);
		sub_103C16E0(&crc, v1 + 48, 4);
		sub_103C16E0(&crc, v1 + 52, 4);
		sub_103C16E0(&crc, v1 + 56, 2);
		sub_103C16E0(&crc, v1 + 58, 2);
		CRC32_Final(&crc);
	*/

	CRC32_Init(&crc);
	CRC32_ProcessBuffer(&crc, &command_number, sizeof(command_number));
	CRC32_ProcessBuffer(&crc, &tick_count, sizeof(tick_count));
	CRC32_ProcessBuffer(&crc, &viewangles, sizeof(viewangles));
	// CRC32_ProcessBuffer(&crc, &aimdirection, sizeof(aimdirection));
	CRC32_ProcessBuffer(&crc, &forwardmove, sizeof(forwardmove));
	CRC32_ProcessBuffer(&crc, &sidemove, sizeof(sidemove));
	CRC32_ProcessBuffer(&crc, &upmove, sizeof(upmove));
	CRC32_ProcessBuffer(&crc, &buttons, sizeof(buttons));
	CRC32_ProcessBuffer(&crc, &impulse, sizeof(impulse));
	CRC32_ProcessBuffer(&crc, &weaponselect, sizeof(weaponselect));
	CRC32_ProcessBuffer(&crc, &weaponsubtype, sizeof(weaponsubtype));
	CRC32_ProcessBuffer(&crc, &random_seed, sizeof(random_seed));
	CRC32_ProcessBuffer(&crc, &mousedx, sizeof(mousedx));
	CRC32_ProcessBuffer(&crc, &mousedy, sizeof(mousedy));
	// CRC32_ProcessBuffer(&crc, &hasbeenpredicted, sizeof(hasbeenpredicted));
	// CRC32_ProcessBuffer(&crc, &headangles, sizeof(headangles));
	// CRC32_ProcessBuffer(&crc, &headoffset, sizeof(headoffset));
	CRC32_Final(&crc);

	return crc;
}
