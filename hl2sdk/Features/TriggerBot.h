#pragma once
#include "BaseFeatures.h"

class CTriggerBot : public CBaseFeatures
{
public:
	CTriggerBot();
	~CTriggerBot();

	virtual void OnCreateMove(CUserCmd* cmd, bool* bSendPacket) override;
	virtual void OnMenuDrawing() override;
	virtual void OnEnginePaint(PaintMode_t mode) override;

	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;

	CBasePlayer* GetAimTarget(const QAngle& eyeAngles);
	static bool IsValidTarget(CBasePlayer* entity);
	bool HasValidWeapon(CBaseWeapon* weapon);
	bool HasShotgun(CBaseWeapon* weapon);

	bool IsVisableToPosition(CBasePlayer* local, CBasePlayer* target, const Vector& position);

private:	// 菜单项
	bool m_bActive = false;
	bool m_bCrosshairs = false;
	bool m_bNonWitch = true;
	bool m_bBlockFriendlyFire = true;
	bool m_bAimPosition = false;
	bool m_bVelExt = true;
	bool m_bForwardtrack = false;

	bool m_bTraceHead = false;
	bool m_bTraceSilent = true;
	bool m_bTraceVelExt = true;
	bool m_bTraceForwardtrack = false;
	float m_fTraceFov = 9.0f;
	bool m_bTraceVisible = true;
	bool m_bTraceWithoutMagnum = true;
	bool m_bTraceShotgunChest = true;

	bool m_bFollowEnemy = false;
	float m_fFollowFov = 9.0f;
	bool m_bFollowVisible = true;

private:
	CBasePlayer* m_pAimTarget = nullptr;
	int m_iHitBox = 0;
	int m_iHitGroup = 0;
	Vector m_vecAimOrigin;
};

extern CTriggerBot* g_pTriggerBot;
