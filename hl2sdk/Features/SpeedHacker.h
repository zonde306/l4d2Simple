#pragma once
#include "BaseFeatures.h"

class CSpeedHacker : public CBaseFeatures
{
public:
	CSpeedHacker();
	~CSpeedHacker();
	
	virtual void OnMenuDrawing() override;
	virtual void OnCreateMove(CUserCmd* cmd, bool*) override;

	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;

	void RunPositionAdjustment(CUserCmd* cmd);

private:
	float m_fOriginSpeed = 1.0f;
	float m_fUseSpeed = 1.0f;
	float m_fFireSpeed = 1.0f;
	float m_fWalkSpeed = 1.0f;
	bool m_bActive = false;
	bool m_bPositionAdjustment = false;
};

extern CSpeedHacker* g_pSpeedHacker;
