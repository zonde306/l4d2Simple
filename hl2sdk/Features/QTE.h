#pragma once
#include "BaseFeatures.h"
#include <optional>
#include <chrono>
#include <unordered_set>
#include <atomic>

class CQuickTriggerEvent : public CBaseFeatures
{
public:
	CQuickTriggerEvent();
	~CQuickTriggerEvent();

	virtual void OnCreateMove(CUserCmd* cmd, bool*) override;

	virtual void OnMenuDrawing() override;
	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;

	virtual bool OnEmitSound(std::string& sample, int& entity, int& channel, float& volume,
		SoundLevel_t& level, int& flags, int& pitch, Vector& origin, Vector& direction,
		bool& updatePosition, float& soundTime) override;

private:
	void HandleShotSelfClear(CBasePlayer* self, CBasePlayer* enemy, CUserCmd* cmd, float distance = -1.0f);
	void HandleMeleeSelfClear(CBasePlayer* self, CBasePlayer* enemy, CUserCmd* cmd, float distance = -1.0f, CBaseWeapon* weapon = nullptr);
	void HandleShoveSelfClear(CBasePlayer* self, CBasePlayer* enemy, CUserCmd* cmd, float distance = -1.0f);
	void HandleWitchCrown(CBasePlayer* self, CBasePlayer* enemy, CUserCmd* cmd, float distance = -1.0f);

	QAngle GetAimAngles(CBasePlayer* self, CBasePlayer* enemy, std::optional<bool> visable = {});
	bool SetAimAngles(CUserCmd* cmd, QAngle& aimAngles, bool tick = false, bool melee = false);
	bool IsVisibleEnemy(CBasePlayer* local, CBasePlayer* enemy, const Vector& start, const Vector& end);
	bool HasShotgun(CBaseWeapon* weapon);
	Vector GetTargetAimPosition(CBasePlayer* entity, std::optional<bool> visible = {});
	CBasePlayer* FindTarget(CBasePlayer* local, const QAngle& myEyeAngles);
	std::unordered_set<CBasePlayer*> GetAllTongueSmoker();

private:
	bool m_bActive = false;
	bool m_bVelExt = true;
	bool m_bLagExt = true;
	bool m_bSwingExt = true;

	bool m_bOnlyVisible = true;
	bool m_bPerfectSilent = true;
	bool m_bSilent = true;
	bool m_bCheckFov = true;
	int m_iShoveTicks = 6;
	bool m_bMeleeAsShove = true;
	bool m_bMeleeUnslienced = true;
	float m_fMeleeUnsliencedFov = 90.0f;

	bool m_bSmoker = true;
	bool m_bHunter = true;
	bool m_bJockey = true;
	bool m_bCharger = true;
	bool m_bWitch = true;
	bool m_bBoomer = true;
	bool m_bRock = true;
	bool m_bLogInfo = false;

	float m_fHunterDistance = 125.0f;
	float m_fJockeyDistance = 150.0f;
	float m_fChargerDistance = 150.0f;
	float m_fWitchDistance = 50.0f;
	float m_fRockDistance = 350.0f;
	float m_fSmokerDistance = 750.0f;

	float m_fShoveDstExtra = 40.0f;
	float m_fMeleeDstExtra = 50.0f;
	
	float m_fHunterFov = 25.0f;
	float m_fJockeyFov = 40.0f;

	float m_fJockeyScale = 2.0f;
	float m_fMeleeScale = 2.0f;

	bool m_bCanShotSmoker = true;
	bool m_bCanShotHunter = true;
	bool m_bCanShotJockey = true;
	bool m_bCanShotCharger = false;
	bool m_bCanShotRock = true;
	bool m_bCanMeleeSmoker = true;
	bool m_bCanMeleeHunter = true;
	bool m_bCanMeleeJockey = true;
	bool m_bCanMeleeCharger = true;
	bool m_bCanMeleeRock = true;
	bool m_bCanShoveHunter = true;
	bool m_bCanShoveJockey = true;
	bool m_bCanShoveSmoker = true;

	std::chrono::system_clock::time_point m_StartAttackTime[65];
	CBasePlayer* m_pSmokerAttacker = nullptr;
	std::chrono::system_clock::time_point m_WaitTime;
	
	enum MeleeAttackState_t
	{
		MAS_None,
		MAS_PreAttack,	// 开始摆动
		MAS_Attacking,	// 前摇
		MAS_PostAttack,	// 造成伤害
	};
	MeleeAttackState_t m_iMeleeState = MAS_None;
};

extern CQuickTriggerEvent* g_pQTE;
