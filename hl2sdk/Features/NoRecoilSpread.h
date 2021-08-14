﻿#pragma once
#include "BaseFeatures.h"

class CVM_ShotgunSound;

class CViewManager : public CBaseFeatures
{
public:
	CViewManager();
	~CViewManager();

	void OnCreateMove(CUserCmd* cmd, bool* bSendPacket) override;
	virtual void OnFrameStageNotify(ClientFrameStage_t stage) override;
	virtual void OnMenuDrawing() override;
	virtual void OnEnginePaint(PaintMode_t mode);

	virtual void OnConfigLoading(CProfile& cfg) override;
	virtual void OnConfigSave(CProfile& cfg) override;
	virtual void OnConnect() override;
	virtual void OnDisconnect() override;
	virtual void OnGameEventClient(IGameEvent* event) override;
	virtual void OnGameEvent(IGameEvent* event, bool dontBroadcast) override;

	bool ApplySilentAngles(const QAngle& viewAngles, int ticks = 1);
	bool ApplySilentFire(const QAngle& viewAngles);

	void RunNoRecoilSpread(CUserCmd* cmd, CBaseWeapon* weapon, bool* bSendPacket);

	bool StartSilent(CUserCmd* cmd);
	bool FinishSilent(CUserCmd* cmd);

	void RemoveSpread(CUserCmd* cmd);
	void RemoveRecoil(CUserCmd* cmd);
	void SmoothAngles(CUserCmd* cmd, bool* bSendPacket);

	void RunRapidFire(CUserCmd* cmd, CBasePlayer* local, CBaseWeapon* weapon);
	void RunSilentAngles(CUserCmd* cmd, bool* bSendPacket, bool canFire);

	bool IsUsingMinigun(CBasePlayer* player);

private:
	bool m_bNoSpread = true;
	bool m_bNoRecoil = true;
	bool m_bNoVisRecoil = true;
	bool m_bRapidFire = true;
	bool m_bSilentNoSpread = true;
	bool m_bRealAngles = false;
	bool m_bFakeAngleBug = false;
	bool m_bFakeLag = false;
	float m_fSpreadFactor = 1.0f;
	float m_fRecoilFactor = 1.0f;
	int m_iSmoothTicks = 0;

private:
	bool m_bSilentFire = false;
	int m_iSilentTicks = -1;
	QAngle m_vecSilentAngles = INVALID_VECTOR;

private:
	QAngle m_vecAngles = INVALID_VECTOR;
	QAngle m_vecPunchAngle = INVALID_VECTOR;
	float m_fSideMove = 0.0f;
	float m_fForwardMove = 0.0f;
	float m_fUpMove = 0.0f;

private:
	Vector m_vecPunch;
	Vector m_vecSpread;
	QAngle m_vecViewAngles;
	bool m_bHasFiring = false;
	bool m_bHasSilent = false;
	int m_iPacketBlocked = 0;
	int m_iKeepTicks = 0;
	bool m_bLastFired = false;
	QAngle m_vecLastFiredAngles = INVALID_VECTOR;

private:
	bool m_bRapidIgnore = true;
	CVM_ShotgunSound* m_pEventListener = nullptr;
};

class CVM_ShotgunSound : public IGameEventListener2
{
public:
	friend CViewManager;
	virtual void FireGameEvent(IGameEvent* event) override;

protected:
	bool HasShotgun(CBaseWeapon* weapon);

private:
	bool m_bShotgunSound = true;
	bool m_bIsGameTick = false;
};

extern CViewManager* g_pViewManager;
