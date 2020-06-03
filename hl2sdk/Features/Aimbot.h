﻿#pragma once
#include "BaseFeatures.h"
#include <optional>
#include <queue>
#include <chrono>

class CAimBot : public CBaseFeatures
{
public:
	CAimBot();
	~CAimBot();

	struct QueuedTarget_t
	{
		int priority = 0;
		CBasePlayer* target = nullptr;
		float fov = 0.0f;
		float distance = 0.0f;

		inline bool operator<(const QueuedTarget_t& t)
		{
			return priority < t.priority;
		}

		inline bool operator>(const QueuedTarget_t& t)
		{
			return priority > t.priority;
		}
	};

	virtual void OnCreateMove(CUserCmd* cmd, bool* bSendPacket) override;
	virtual void OnMenuDrawing() override;
	virtual void OnEnginePaint(PaintMode_t mode) override;
	virtual void OnFrameStageNotify(ClientFrameStage_t stage) override;

	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;

	CBasePlayer* FindTarget(const QAngle& myEyeAngles);
	std::priority_queue<QueuedTarget_t> ScanTarget(const QAngle& myEyeAngles);

	bool IsTargetVisible(CBasePlayer* entity, Vector aimPosition = NULL_VECTOR);
	bool IsValidTarget(CBasePlayer* entity);
	bool HasValidWeapon(CBaseWeapon* weapon);
	bool HasShotgun(CBaseWeapon* weapon);
	Vector GetTargetAimPosition(CBasePlayer* entity, std::optional<bool> visible = {});

	bool CanRunAimbot(CBasePlayer* entity);
	Vector GetAimPosition(CBasePlayer* local, const Vector& eyePosition, CBasePlayer** hitEntity = nullptr);
	bool IsFatalTarget(CBasePlayer* entity);
	
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
	bool m_bVelExt = true;
	bool m_bForwardtrack = false;
	bool m_bShowTarget = false;

	float m_fAimFov = 30.0f;
	float m_fAimDist = 3000.0f;
	bool m_bShotgunChest = true;
	bool m_bFatalFirst = true;

private:
	CBasePlayer* m_pAimTarget = nullptr;
	QAngle m_vecAimAngles;
	bool m_bRunAutoAim = false;

	float m_fTargetFov = 0.0f;
	float m_fTargetDistance = 0.0f;
	int m_iEntityIndex = -1;
};

extern CAimBot* g_pAimbot;
