#include "usercmd.h"

CRC32_t CUserCmd::GetChecksum() const
{
	CRC32_t crc;

	CRC32_Init(&crc);

	/*
	CRC32_ProcessBuffer(&crc, &command_number, sizeof(command_number));
	CRC32_ProcessBuffer(&crc, &tick_count, sizeof(tick_count));
	CRC32_ProcessBuffer(&crc, &viewangles, sizeof(viewangles));
	// CRC32_ProcessBuffer(&crc, &aimdirection, sizeof(aimdirection));
	// CRC32_ProcessBuffer(&crc, &forwardmove, sizeof(forwardmove));
	CRC32_ProcessBuffer(&crc, &sidemove, sizeof(sidemove));
	CRC32_ProcessBuffer(&crc, &upmove, sizeof(upmove));
	CRC32_ProcessBuffer(&crc, &buttons, sizeof(buttons));
	// CRC32_ProcessBuffer(&crc, &impulse, sizeof(impulse));
	// CRC32_ProcessBuffer(&crc, &weaponselect, sizeof(weaponselect));
	// CRC32_ProcessBuffer(&crc, &weaponsubtype, sizeof(weaponsubtype));
	CRC32_ProcessBuffer(&crc, &random_seed, sizeof(random_seed));
	// CRC32_ProcessBuffer(&crc, &mousedx, sizeof(mousedx));
	// CRC32_ProcessBuffer(&crc, &mousedy, sizeof(mousedy));
	// CRC32_ProcessBuffer( &crc, &hasbeenpredicted, sizeof( hasbeenpredicted ) );
	// CRC32_ProcessBuffer( &crc, &headangles, sizeof( headangles ) );
	// CRC32_ProcessBuffer( &crc, &headoffset, sizeof( headoffset ) );
	*/

	CRC32_ProcessBuffer(&crc, &command_number, sizeof(command_number));
	CRC32_ProcessBuffer(&crc, &tick_count, sizeof(tick_count));
	CRC32_ProcessBuffer(&crc, &viewangles, sizeof(viewangles));
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

	CRC32_Final(&crc);

	return crc;
}
