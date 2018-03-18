#include "Knifebot.h"
#include "../hook.h"

CKnifeBot* g_pKnifeBot = nullptr;

CKnifeBot::CKnifeBot() : CBaseFeatures::CBaseFeatures()
{
}

CKnifeBot::~CKnifeBot()
{
	CBaseFeatures::~CBaseFeatures();
}

void CKnifeBot::OnCreateMove(CUserCmd * cmd, bool *)
{
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive() || player->GetTeam() != 2)
		return;

	CBaseWeapon* weapon = player->GetActiveWeapon();
	if (weapon == nullptr)
		return;
	
	int weaponId = weapon->GetWeaponID();
	float nextAttack = weapon->GetNextPrimary();
	float serverTime = g_pClientPrediction->GetServerTime();

	if (m_bFastMelee)
	{
		if (RunFastMelee(cmd, weaponId, nextAttack, serverTime))
			return;
	}

	if (m_bAutoFire)
	{
		if (weaponId == Weapon_Melee && nextAttack <= serverTime)
		{
			cmd->buttons |= IN_ATTACK;
			return;
		}
	}

	if (m_bAutoShov)
	{
		nextAttack = weapon->GetSecondry();
		if (nextAttack <= serverTime)
		{
			cmd->buttons |= IN_ATTACK2;
			return;
		}
	}
}

void CKnifeBot::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("KnifeBot")))
		return;

	ImGui::Checkbox(XorStr("Auto Melee"), &m_bAutoFire);
	ImGui::Checkbox(XorStr("Auto Shov"), &m_bAutoShov);
	ImGui::Checkbox(XorStr("Melee Faster"), &m_bFastMelee);

	ImGui::TreePop();
}

bool CKnifeBot::RunFastMelee(CUserCmd* cmd, int weaponId, float nextAttack, float serverTime)
{
	switch (m_eMeleeStage)
	{
		case FMS_None:
		{
			if (weaponId == Weapon_Melee && nextAttack <= serverTime)
			{
				cmd->buttons |= IN_ATTACK;
				m_eMeleeStage = FMS_Primary;
				return true;
			}
			else if (m_iMeleeTick > 0)
				m_iMeleeTick = 0;

			break;
		}
		case FMS_Primary:
		{
			if (weaponId == Weapon_Melee && nextAttack > serverTime)
			{
				g_pClientInterface->Engine->ClientCmd_Unrestricted(XorStr("lastinv"));
				m_eMeleeStage = FMS_Primary;
				return true;
			}
			else
				m_iMeleeTick += 1;

			break;
		}
		case FMS_Secondary:
		{
			if (weaponId != Weapon_Melee)
			{
				g_pClientInterface->Engine->ClientCmd_Unrestricted(XorStr("lastinv"));
				m_eMeleeStage = FMS_None;
				return true;
			}
			else
				m_iMeleeTick += 1;

			break;
		}
	}

	if (m_iMeleeTick >= 10)
	{
		m_iMeleeTick = 0;
		m_eMeleeStage = FMS_None;
	}

	return false;
}
