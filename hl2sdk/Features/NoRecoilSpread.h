#pragma once
#include "BaseFeatures.h"

class CViewAnglesManager
{
public:
	CViewAnglesManager();

	bool StartSilent(CUserCmd* cmd);
	bool FinishSilent(CUserCmd* cmd);

	void RemoveSpread(CUserCmd* cmd);
	void RemoveRecoil(CUserCmd* cmd);

private:
	QAngle m_vecAngles;
	float m_fSideMove = 0.0f;
	float m_fForwardMove = 0.0f;
	float m_fUpMove = 0.0f;
};

extern CViewAnglesManager* g_pViewAnglesManager;
