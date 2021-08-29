#pragma once

#include "../Utils/checksum_crc.h"

#include <cstdint>
#include <Windows.h>

struct player_info_t
{
public:
	uint64_t networkId;
	char name[MAX_PLAYER_NAME_LENGTH];
	int32_t userId;
	char steamId[SIGNED_GUID_LEN + 1];
	uint32_t friendsId;
	char friendsName[MAX_PLAYER_NAME_LENGTH];
	bool isBot;
	bool isHltv;
	CRC32_t customFiles[MAX_CUSTOM_FILES];
	unsigned char filesDownloaded;
};