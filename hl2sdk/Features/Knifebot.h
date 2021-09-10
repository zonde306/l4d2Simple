#pragma once
#include "BaseFeatures.h"
#include <set>

class CKnifeBot : public CBaseFeatures
{
public:
	CKnifeBot();
	~CKnifeBot();

	virtual void OnCreateMove(CUserCmd* cmd, bool*) override;
	virtual void OnMenuDrawing() override;

	virtual void OnConfigLoading(CProfile& cfg) override;
	virtual void OnConfigSave(CProfile& cfg) override;
	virtual void OnEnginePaint(PaintMode_t) override;

	bool CheckMeleeAttack(const QAngle& myEyeAngles);
	bool RunFastMelee(CUserCmd* cmd, int weaponId, float nextAttack, float serverTime);
	bool HasEnemyVisible(CBasePlayer* entity, const Vector& position);
	bool IsShoveReady(CBasePlayer* player, CBaseWeapon* weapon);

private:
	bool m_bAutoFire = false;
	bool m_bAutoShove = false;
	bool m_bFastMelee = true;
	bool m_bVelExt = true;
	bool m_bForwardtrack = false;
	bool m_bVisualOnly = true;

	float m_fExtraShoveRange = 0.0f;
	float m_fExtraMeleeRange = 9.0f;
	float m_fShoveFOV = 90.0f;
	float m_fMeleeFOV = 70.0f;

	bool m_bDebug = false;
	int m_iSequence = -1;
	std::set<int> m_CommonStaggering;

private:
	enum FastMeleeStage_t
	{
		FMS_None = 0,
		FMS_Primary,
		FMS_Secondary
	};

	FastMeleeStage_t m_eMeleeStage = FMS_None;
	int m_iMeleeTick = 0;

	bool m_bCanMeleeAttack = false;
	bool m_bCanShoveAttack = false;
};

extern CKnifeBot* g_pKnifeBot;
