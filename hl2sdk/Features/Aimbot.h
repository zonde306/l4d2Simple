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

	CBasePlayer* FindTarget(const QAngle& myEyeAngles);
	bool IsTargetVisible(CBasePlayer* entity);
	bool IsValidTarget(CBasePlayer* entity);
	bool HasValidWeapon(CBaseWeapon* weapon);

	Vector GetAimPosition(CBasePlayer* local, const Vector& eyePosition, CBasePlayer** hitEntity = nullptr);

private:	// 菜单项
	bool m_bActive = false;
	bool m_bOnFire = true;

	bool m_bVisible = false;
	bool m_bSilent = false;
	bool m_bPerfectSilent = false;
	bool m_bNonFriendly = true;
	bool m_bDistance = false;
	bool m_bNonWitch = true;
	bool m_bShowRange = false;
	bool m_bShowAngles = false;

	float m_fAimFov = 30.0f;
	float m_fAimDist = 3000.0f;

private:
	CBasePlayer* m_pAimTarget = nullptr;
	QAngle m_vecAimAngles;
	bool m_bRunAutoAim = false;
};

extern CAimBot* g_pAimbot;
