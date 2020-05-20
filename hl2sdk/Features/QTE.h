#pragma once
#include "BaseFeatures.h"
#include <optional>

class CQuickTriggerEvent : public CBaseFeatures
{
public:
	CQuickTriggerEvent();
	~CQuickTriggerEvent();

	virtual void OnCreateMove(CUserCmd* cmd, bool*) override;

	virtual void OnMenuDrawing() override;
	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;

private:
	void HandleShotSelfClear(CBasePlayer* self, CBasePlayer* enemy, CUserCmd* cmd, float distance = -1.0f);
	void HandleMeleeSelfClear(CBasePlayer* self, CBasePlayer* enemy, CUserCmd* cmd, float distance = -1.0f);
	void HandleShoveSelfClear(CBasePlayer* self, CBasePlayer* enemy, CUserCmd* cmd, float distance = -1.0f);
	void HandleWitchCrown(CBasePlayer* self, CBasePlayer* enemy, CUserCmd* cmd, float distance = -1.0f);

	QAngle GetAimAngles(CBasePlayer* self, CBasePlayer* enemy, std::optional<bool> visable = {});
	void SetAimAngles(CUserCmd* cmd, QAngle& aimAngles);
	bool IsVisibleEnemy(CBasePlayer* local, CBasePlayer* enemy, const Vector& start, const Vector& end);
	bool HasShotgun(CBaseWeapon* weapon);
	Vector GetTargetAimPosition(CBasePlayer* entity, std::optional<bool> visible = {});

private:
	bool m_bActive = false;
	bool m_bVelExt = true;
	bool m_bLagExt = true;

	bool m_bOnlyVisible = true;
	bool m_bPerfectSilent = true;
	bool m_bSilent = true;

	bool m_bSmoker = true;
	bool m_bHunter = true;
	bool m_bJockey = true;
	bool m_bCharger = true;
	bool m_bWitch = true;
	bool m_bBoomer = true;
	bool m_bRock = true;

	float m_fHunterDistance = 400.0f;
	float m_fJockeyDistance = 300.0f;
	float m_fChargerDistance = 500.0f;
	float m_fWitchDistance = 250.0f;
	float m_fRockDistance = 350.0f;

	float m_fShoveDstExtra = 25.0f;
	float m_fMeleeDstExtra = 30.0f;
};

extern CQuickTriggerEvent* g_pQTE;
