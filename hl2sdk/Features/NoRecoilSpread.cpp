#include "NoRecoilSpread.h"
#include "../hook.h"

CViewManager* g_pViewManager = nullptr;

#define IsSingleWeapon(_id)			(_id == Weapon_Pistol || _id == Weapon_ShotgunPump || _id == Weapon_ShotgunAuto || _id == Weapon_SniperHunting || _id == Weapon_ShotgunChrome || _id == Weapon_SniperMilitary || _id == Weapon_ShotgunSpas || _id == Weapon_PistolMagnum || _id == Weapon_SniperAWP || _id == Weapon_SniperScout)

CViewManager::CViewManager() : CBaseFeatures::CBaseFeatures()
{
	m_vecAngles.Invalidate();
}

CViewManager::~CViewManager()
{
	CBaseFeatures::~CBaseFeatures();
}

void CViewManager::OnCreateMove(CUserCmd * cmd, bool * bSendPacket)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return;

	CBaseWeapon* weapon = local->GetActiveWeapon();
	if (weapon == nullptr)
		return;

	if (m_bHasApplySilent)
	{
		StartSilent(cmd);
		cmd->viewangles = m_vecSilentAngles;
		*bSendPacket = false;
	}
	else
	{
		FinishSilent(cmd);
		*bSendPacket = true;
	}

	if (m_bNoRecoil)
		RemoveRecoil(cmd);

	if (m_bNoSpread)
		RemoveSpread(cmd);

	if (m_bRapidFire)
		RunRapidFire(cmd, local, weapon);

	m_bHasApplySilent = false;
}

void CViewManager::OnFrameStageNotify(ClientFrameStage_t stage)
{
	if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return;

	if (m_bNoVisRecoil)
		local->GetPunch().SetZero();
}

void CViewManager::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("AimHelper")))
		return;

	ImGui::Checkbox(XorStr("No Recoil"), &m_bNoRecoil);
	ImGui::Checkbox(XorStr("No Visual Recoil"), &m_bNoVisRecoil);
	ImGui::Checkbox(XorStr("No Spread"), &m_bNoSpread);
	ImGui::Checkbox(XorStr("Rapid Fire"), &m_bRapidFire);

	ImGui::TreePop();
}

bool CViewManager::ApplySilentAngles(const QAngle & viewAngles)
{
	if (m_bHasApplySilent)
		return false;
	
	m_bHasApplySilent = true;
	m_vecSilentAngles = viewAngles;
	return true;
}

bool CViewManager::StartSilent(CUserCmd * cmd)
{
	if (m_vecAngles.IsValid())
		return false;

	m_vecAngles = cmd->viewangles;
	m_fSideMove = cmd->sidemove;
	m_fForwardMove = cmd->forwardmove;
	m_fUpMove = cmd->upmove;

	return true;
}

bool CViewManager::FinishSilent(CUserCmd * cmd)
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

void CViewManager::RemoveSpread(CUserCmd * cmd)
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

void CViewManager::RemoveRecoil(CUserCmd * cmd)
{
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive())
		return;

	Vector punch = player->GetPunch();
	cmd->viewangles.x -= punch.x;
	cmd->viewangles.y -= punch.y;
}

void CViewManager::RunRapidFire(CUserCmd* cmd, CBasePlayer* local, CBaseWeapon* weapon)
{
	if (!(cmd->buttons & IN_ATTACK))
		return;

	int classId = weapon->GetClassID();
	if ((local->GetTeam() == 3 || IsSingleWeapon(classId)) &&
		!m_bRapidIgnore && weapon->GetPrimary() <= 0.0f)
	{
		cmd->buttons &= ~IN_ATTACK;
		m_bRapidIgnore = true;
	}
	else
	{
		m_bRapidIgnore = false;
	}
}