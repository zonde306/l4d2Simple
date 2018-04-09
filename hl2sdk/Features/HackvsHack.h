#pragma once
#include "BaseFeatures.h"

class CHackVSHack : public CBaseFeatures
{
public:
	CHackVSHack();
	~CHackVSHack();

	virtual void OnCreateMove(CUserCmd*, bool*) override;
	virtual void OnMenuDrawing() override;

	void RunLegitAntiAim(CUserCmd* cmd, bool* bSendPacket);
	void RunAirStuck(CUserCmd* cmd);
	void RunTeleport(CUserCmd* cmd);

private:
	bool m_bLegitAntiAim = false;
	bool m_bAirStuck = false;
	bool m_bTeleport = false;

private:
	bool m_bFlip = false;
	int m_iChockedPacket = -1;
};

extern CHackVSHack* g_pHackVsHack;
