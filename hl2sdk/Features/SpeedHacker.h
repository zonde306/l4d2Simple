#pragma once
#include "BaseFeatures.h"

class CSpeedHacker : public CBaseFeatures
{
public:
	CSpeedHacker();
	~CSpeedHacker();
	
	virtual void OnMenuDrawing() override;
	virtual void OnCreateMove(CUserCmd* cmd, bool*) override;

private:
	float m_fOriginSpeed = 1.0f;
	float m_fUseSpeed = 1.0f;
	float m_fFireSpeed = 1.0f;
	float m_fWalkSpeed = 1.0f;
	bool m_bActive = false;
};

extern CSpeedHacker* g_pSpeedHacker;
