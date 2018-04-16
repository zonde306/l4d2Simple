#pragma once
#include "BaseFeatures.h"

class CAimBot : public CBaseFeatures
{
public:
	CAimBot();
	~CAimBot();

	virtual void OnCreateMove(CUserCmd* cmd, bool* bSendPacket) override;
	virtual void OnMenuDrawing() override;
	virtual void OnEnginePaint(PaintMode_t mode) override;
	virtual void OnFrameStageNotify(ClientFrameStage_t stage) override;

	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;

	CBasePlayer* FindTarget(const QAngle& myEyeAngles);
	bool IsTargetVisible(CBasePlayer* entity);
	bool IsValidTarget(CBasePlayer* entity);
	bool HasValidWeapon(CBaseWeapon* weapon);

	bool CanRunAimbot(CBasePlayer* entity);
	Vector GetAimPosition(CBasePlayer* local, const Vector& eyePosition, CBasePlayer** hitEntity = nullptr);

private:	// 菜单项
	bool m_bActive = false;
	bool m_bOnFire = true;

	bool m_bVisible = true;
	bool m_bSilent = true;
	bool m_bPerfectSilent = true;
	bool m_bNonFriendly = true;
	bool m_bDistance = true;
	bool m_bNonWitch = true;
	bool m_bShowRange = false;
	bool m_bShowAngles = false;

	float m_fAimFov = 30.0f;
	float m_fAimDist = 3000.0f;

private:
	CBasePlayer* m_pAimTarget = nullptr;
	QAngle m_vecAimAngles;
	bool m_bRunAutoAim = false;

	float m_fTargetFov = 0.0f;
	float m_fTargetDistance = 0.0f;
};

extern CAimBot* g_pAimbot;
