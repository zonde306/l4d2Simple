#pragma once
#include "BaseFeatures.h"

class CKnifeBot : public CBaseFeatures
{
public:
	CKnifeBot();
	~CKnifeBot();

	virtual void OnCreateMove(CUserCmd* cmd, bool*) override;
	virtual void OnMenuDrawing() override;

	virtual void OnConfigLoading(const config_type& data) override;
	virtual void OnConfigSave(config_type& data) override;

	bool CheckMeleeAttack(const QAngle& myEyeAngles);
	bool RunFastMelee(CUserCmd* cmd, int weaponId, float nextAttack, float serverTime);
	bool HasEnemyVisible(CBasePlayer* entity, const Vector& position);

private:
	bool m_bAutoFire = false;
	bool m_bAutoShove = false;
	bool m_bFastMelee = true;
	bool m_bVelExt = true;
	bool m_bForwardtrack = false;

	float m_fExtraShoveRange = 0.0f;
	float m_fExtraMeleeRange = 0.0f;

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
