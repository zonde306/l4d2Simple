#pragma once
#include <cinttypes>

class CViewSetup
{
public:
	char pad_0000[4]; //0x0000
	uint32_t N00000050; //0x0004
	uint32_t N00000051; //0x0008
	uint32_t N00000052; //0x000C
	uint32_t width; //0x0010
	uint32_t unscale_width; //0x0014
	uint32_t height; //0x0018
	uint32_t unscale_height; //0x001C
	bool N00000057; //0x0020
	char pad_0021[3]; //0x0021
	float N00000058; //0x0024
	float N00000059; //0x0028
	float N0000005A; //0x002C
	float N0000005B; //0x0030
	float m_fov; //0x0034
	float m_viewmodelfov; //0x0038
	char pad_003C[2052]; //0x003C
}; //Size: 0x0840

