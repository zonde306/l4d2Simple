#include "HackvsHack.h"
#include "../hook.h"

CHackVSHack* g_pHackVsHack = nullptr;

CHackVSHack::CHackVSHack() : CBaseFeatures::CBaseFeatures()
{
}

CHackVSHack::~CHackVSHack()
{
	CBaseFeatures::~CBaseFeatures();
}

void CHackVSHack::OnCreateMove(CUserCmd* cmd, bool* bSendPacket)
{
	if (!(*bSendPacket) || m_bMenuOpen)
		return;
	
	if (m_bAirStuck)
		RunAirStuck(cmd);

	if (m_bTeleport)
		RunTeleport(cmd);

	if (m_bLegitAntiAim)
		RunLegitAntiAim(cmd, bSendPacket);
}

void CHackVSHack::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("Hack vs Hack")))
		return;

	ImGui::Checkbox(XorStr("Legit AntiAim"), &m_bLegitAntiAim);
	IMGUI_TIPS("反自动瞄准，防止被自瞄爆头。");

	ImGui::Checkbox(XorStr("Air Stuck"), &m_bAirStuck);
	IMGUI_TIPS("在空中卡住不动，按住 E 启动。");

	ImGui::Checkbox(XorStr("Teleport"), &m_bTeleport);
	IMGUI_TIPS("传送到地图外面，按 R 启动。用于特感传送刷CD。");

	ImGui::TreePop();
}

void CHackVSHack::OnDisconnect()
{
	m_bFlip = false;
	m_iChockedPacket = -1;
}

void CHackVSHack::RunLegitAntiAim(CUserCmd * cmd, bool * bSendPacket)
{
	if ((cmd->buttons & IN_USE) || (cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2))
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive() || local->GetMoveType() == MOVETYPE_LADDER)
		return;

	if (++m_iChockedPacket >= 1)
	{
		*bSendPacket = false;
		
		if (m_bFlip)
			cmd->viewangles.y += 90.0f;
		else
			cmd->viewangles.y -= 90.0f;

		m_iChockedPacket = -1;
	}

	m_bFlip = !m_bFlip;
}

void CHackVSHack::RunAirStuck(CUserCmd * cmd)
{
	if (!(cmd->buttons & IN_USE) || (cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2))
		return;
	
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive() || (g_pClientPrediction->GetFlags() & FL_ONGROUND))
		return;

	// 使用 0xFFFFF 或者 16777216
	cmd->tick_count = INT_MAX;
}

void CHackVSHack::RunTeleport(CUserCmd * cmd)
{
	if (!(cmd->buttons & IN_RELOAD) || (cmd->buttons & IN_ATTACK) || (cmd->buttons & IN_ATTACK2))
		return;
	
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return;

	cmd->viewangles.z = 9e+37f;
}
