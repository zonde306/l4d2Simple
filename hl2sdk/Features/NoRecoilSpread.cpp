#include "NoRecoilSpread.h"
#include "../hook.h"

CViewAnglesManager* g_pViewAnglesManager = nullptr;

CViewAnglesManager::CViewAnglesManager()
{
	m_vecAngles.Invalidate();
}

bool CViewAnglesManager::StartSilent(CUserCmd * cmd)
{
	if (m_vecAngles.IsValid())
		return false;

	m_vecAngles = cmd->viewangles;
	m_fSideMove = cmd->sidemove;
	m_fForwardMove = cmd->forwardmove;
	m_fUpMove = cmd->upmove;

	return true;
}

bool CViewAnglesManager::FinishSilent(CUserCmd * cmd)
{
	if (!m_vecAngles.IsValid())
		return false;

	cmd->viewangles = m_vecAngles;
	cmd->sidemove = m_fSideMove;
	cmd->forwardmove = m_fForwardMove;
	cmd->upmove = m_fUpMove;

	m_vecAngles.Invalidate();
	return true;
}

void CViewAnglesManager::RemoveSpread(CUserCmd * cmd)
{
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive())
		return;

	CBaseWeapon* weapon = player->GetActiveWeapon();
	if (weapon == nullptr/* || !weapon->CanFire()*/)
		return;

	auto spread = g_pClientPrediction->GetWeaponSpread(cmd->random_seed, weapon->GetSpread());
	cmd->viewangles.x -= spread.first;
	cmd->viewangles.y -= spread.second;
}

void CViewAnglesManager::RemoveRecoil(CUserCmd * cmd)
{
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive())
		return;

	Vector punch = player->GetPunch();
	cmd->viewangles.x -= punch.x;
	cmd->viewangles.y -= punch.y;
}
