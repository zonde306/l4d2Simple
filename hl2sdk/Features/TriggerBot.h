#pragma once
#include "BaseFeatures.h"

class CTriggerBot : public CBaseFeatures
{
public:
	CTriggerBot();
	~CTriggerBot();

	virtual void OnCreateMove(CUserCmd* cmd, bool* bSendPacket) override;
	virtual void OnMenuDrawing() override;
	virtual void OnPaintTraverse(VPANEL panel) override;

	CBasePlayer* GetAimTarget(const QAngle& eyeAngles);

	static bool IsValidTarget(CBasePlayer* entity);

	QAngle RunTrigger(CUserCmd* cmd);

private:	// 菜单项
	bool m_bActive = false;
	bool m_bCrosshairs = false;
	bool m_bAntiSpread = false;
	bool m_bAntiPunch = false;

	bool m_bTraceHead = false;
	bool m_bTraceSilent = false;
	float m_fTraceFov = 9.0f;

	bool m_bFollowEnemy = false;
	float m_fFollowFov = 9.0f;

private:
	CBasePlayer* m_pAimTarget = nullptr;
	int m_iHitBox = 0;
	int m_iHitGroup = 0;
	bool m_bRunning = false;
};

extern CTriggerBot* g_pTriggerBot;
