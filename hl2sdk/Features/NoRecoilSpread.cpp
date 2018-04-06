#include "NoRecoilSpread.h"
#include "../Utils/math.h"
#include "../hook.h"
#include "../../l4d2Simple2/config.h"

CViewManager* g_pViewManager = nullptr;

#define IsSingleWeapon(_id)			(_id == Weapon_Pistol || _id == Weapon_ShotgunPump || _id == Weapon_ShotgunAuto || _id == Weapon_SniperHunting || _id == Weapon_ShotgunChrome || _id == Weapon_SniperMilitary || _id == Weapon_ShotgunSpas || _id == Weapon_PistolMagnum || _id == Weapon_SniperAWP || _id == Weapon_SniperScout)
#define NUM_NEW_COMMAND_BITS		4
#define MAX_NEW_COMMANDS			((1 << NUM_NEW_COMMAND_BITS)-1)

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

	bool canFire = weapon->CanFire();
	m_bHasFiring = !!(cmd->buttons & IN_ATTACK);

	if (m_bSilentFrame)
	{
		StartSilent(cmd);
		cmd->viewangles = m_vecSilentAngles;
		*bSendPacket = false;
	}
	else if(!m_bSilentByFire || !canFire)
	{
		FinishSilent(cmd);
		*bSendPacket = true;
		m_bSilentByFire = false;
	}
	
	if (m_bRapidFire)
		RunRapidFire(cmd, local, weapon);

	// 在不是开枪的情况下不需要调整后坐力和扩散
	if (weapon->IsFireGun() && canFire && m_bHasFiring)
		RunNoRecoilSpread(cmd, weapon, bSendPacket);

	m_bSilentFrame = false;
	m_bHasSilent = !(*bSendPacket);
	m_vecViewAngles = cmd->viewangles;

	if (!(*bSendPacket))
	{
		++m_iPacketBlocked;

		// 修复命令被丢弃的问题
		// bSendPacket 设置为 false 超过上限会导致命令被丢弃
		if (m_iPacketBlocked > MAX_NEW_COMMANDS)
		{
			*bSendPacket = true;
			m_iPacketBlocked = 0;
			Utils::log(XorStr("Warring: choked commands out of range"));
		}
	}

	if (*bSendPacket && m_iPacketBlocked != 0)
		m_iPacketBlocked = 0;
}

void CViewManager::OnFrameStageNotify(ClientFrameStage_t stage)
{
	// if (stage != FRAME_RENDER_START)
	if (stage != FRAME_NET_UPDATE_POSTDATAUPDATE_START)
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return;

	m_vecPunch = local->GetPunch();

	if (m_bNoVisRecoil)
	{
		local->GetPunch().x = 0.0f;
		local->GetPunch().y = 0.0f;
	}
}

void CViewManager::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("Aim Helper")))
		return;

	ImGui::Checkbox(XorStr("No Recoil"), &m_bNoRecoil);
	IMGUI_TIPS("无后坐力。");

	ImGui::Checkbox(XorStr("No Visual Recoil"), &m_bNoVisRecoil);
	IMGUI_TIPS("无后屏幕坐力。");

	ImGui::Checkbox(XorStr("No Spread"), &m_bNoSpread);
	IMGUI_TIPS("子弹无扩散（提升射击精度）。");

	ImGui::Checkbox(XorStr("Rapid Fire"), &m_bRapidFire);
	IMGUI_TIPS("手枪/狙击枪连射（按住开枪键）。");

	ImGui::Checkbox(XorStr("Spread/Recoil Silent"), &m_bSilentNoSpread);
	IMGUI_TIPS("无后坐力/子弹无扩散 防止被观察者看出来。");

	ImGui::Checkbox(XorStr("Real Angles"), &m_bRealAngles);

	ImGui::TreePop();
}

void CViewManager::OnConfigLoading(const config_type & data)
{
	const std::string mainKeys = XorStr("AimHelper");
	
	m_bNoRecoil = g_pConfig->GetBoolean(mainKeys, XorStr("aimhelper_recoil"), m_bNoRecoil);
	m_bNoVisRecoil = g_pConfig->GetBoolean(mainKeys, XorStr("aimhelper_visual_recoil"), m_bNoVisRecoil);
	m_bNoSpread = g_pConfig->GetBoolean(mainKeys, XorStr("aimhelper_spread"), m_bNoSpread);
	m_bRapidFire = g_pConfig->GetBoolean(mainKeys, XorStr("aimhelper_rapid_fire"), m_bRapidFire);
	m_bSilentNoSpread = g_pConfig->GetBoolean(mainKeys, XorStr("aimhelper_silent"), m_bSilentNoSpread);
}

void CViewManager::OnConfigSave(config_type & data)
{
	const std::string mainKeys = XorStr("AimHelper");
	
	g_pConfig->SetValue(mainKeys, XorStr("aimhelper_recoil"), m_bNoRecoil);
	g_pConfig->SetValue(mainKeys, XorStr("aimhelper_visual_recoil"), m_bNoVisRecoil);
	g_pConfig->SetValue(mainKeys, XorStr("aimhelper_spread"), m_bNoSpread);
	g_pConfig->SetValue(mainKeys, XorStr("aimhelper_rapid_fire"), m_bRapidFire);
	g_pConfig->SetValue(mainKeys, XorStr("aimhelper_silent"), m_bSilentNoSpread);
}

void CViewManager::OnEnginePaint(PaintMode_t mode)
{
	if (!m_bRealAngles || !m_bHasFiring)
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return;

	Vector screen;
	Vector aimOrigin = local->GetEyePosition() + m_vecViewAngles.Forward().Scale(100.0f);
	if (!math::WorldToScreenEx(aimOrigin, screen))
		return;

	D3DCOLOR color = CDrawing::ORANGE;
	if (m_bHasSilent)
		color = CDrawing::GREEN;

	g_pDrawing->DrawLine(static_cast<int>(screen.x - 10), static_cast<int>(screen.y - 10),
		static_cast<int>(screen.x + 10), static_cast<int>(screen.y + 10), CDrawing::YELLOW);
	g_pDrawing->DrawLine(static_cast<int>(screen.x + 10), static_cast<int>(screen.y - 10),
		static_cast<int>(screen.x - 10), static_cast<int>(screen.y + 10), CDrawing::YELLOW);
}

bool CViewManager::ApplySilentAngles(const QAngle & viewAngles, bool withFire)
{
	m_bSilentFrame = true;
	m_bSilentByFire = withFire;
	m_vecSilentAngles = viewAngles;
	return true;
}

void CViewManager::RunNoRecoilSpread(CUserCmd * cmd, CBaseWeapon* weapon, bool* bSendPacket)
{
	auto spread = g_pClientPrediction->GetWeaponSpread(cmd->random_seed, weapon);
	m_vecSpread.x = spread.first;
	m_vecSpread.y = spread.second;
	// m_vecSpread.z = 0.0f;

	if (m_bNoRecoil)
		RemoveRecoil(cmd);

	if (m_bNoSpread)
		RemoveSpread(cmd);

	if (m_bSilentNoSpread)
		*bSendPacket = false;
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
	if (!(cmd->buttons & IN_ATTACK))
		return;
	
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive())
		return;

	CBaseWeapon* weapon = player->GetActiveWeapon();
	if (weapon == nullptr/* || !weapon->CanFire()*/)
		return;

	cmd->viewangles.x -= m_vecSpread.x;
	cmd->viewangles.y -= m_vecSpread.y;
}

void CViewManager::RemoveRecoil(CUserCmd * cmd)
{
	if (!(cmd->buttons & IN_ATTACK))
		return;
	
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

	int weaponId = weapon->GetWeaponID();
	if ((local->GetTeam() == 3 || IsSingleWeapon(weaponId)) &&
		!m_bRapidIgnore && weapon->GetPrimaryAttackDelay() <= 0.0f)
	{
		cmd->buttons &= ~IN_ATTACK;
		m_bRapidIgnore = true;
	}
	else
	{
		m_bRapidIgnore = false;
	}
}
