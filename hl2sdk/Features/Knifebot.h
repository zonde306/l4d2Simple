#pragma once
#include "BaseFeatures.h"

class CKnifeBot : public CBaseFeatures
{
public:
	CKnifeBot();
	~CKnifeBot();

	virtual void OnCreateMove(CUserCmd* cmd, bool*) override;
	virtual void OnMenuDrawing() override;

	bool RunFastMelee(CUserCmd* cmd, int weaponId, float nextAttack, float serverTime);

private:
	bool m_bAutoFire = false;
	bool m_bAutoShov = false;
	bool m_bFastMelee = false;

	enum FastMeleeStage_t
	{
		FMS_None = 0,
		FMS_Primary,
		FMS_Secondary
	} m_eMeleeStage;
	int m_iMeleeTick = 0;
};

extern CKnifeBot* g_pKnifeBot;
