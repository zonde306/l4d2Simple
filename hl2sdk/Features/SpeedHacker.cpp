#include "SpeedHacker.h"
#include "../hook.h"
#include "../definitions.h"
#include "../../l4d2Simple2/speedhack.h"
#include "../../l4d2Simple2/config.h"

CSpeedHacker* g_pSpeedHacker = nullptr;

CSpeedHacker::CSpeedHacker() : CBaseFeatures::CBaseFeatures()
{
}

CSpeedHacker::~CSpeedHacker()
{
	CBaseFeatures::~CBaseFeatures();
}

void CSpeedHacker::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("SpeedHack")))
		return;

	ImGui::Checkbox(XorStr("Position Adjustment"), &m_bPositionAdjustment);
	IMGUI_TIPS("tick 优化，一般情况下建议关闭。");

	ImGui::Checkbox(XorStr("SpeedHack Active"), &m_bActive);
	IMGUI_TIPS("加速，勾上后下面的东西才有效果。");

	ImGui::SliderFloat(XorStr("Origin"), &m_fOriginSpeed, 1.0f, 32.0f, XorStr("%.1f"));
	IMGUI_TIPS("默认游戏速度，一般不需要改（默认 1.0）。");

	ImGui::SliderFloat(XorStr("Use (E)"), &m_fUseSpeed, 1.0f, 32.0f, XorStr("%.1f"));
	IMGUI_TIPS("按住 E 时的游戏速度调整（放开会还原）。");

	ImGui::SliderFloat(XorStr("Walk (Shift)"), &m_fWalkSpeed, 1.0f, 32.0f, XorStr("%.1f"));
	IMGUI_TIPS("按住 Shift 时的游戏速度调整（放开会还原）。");

	ImGui::SliderFloat(XorStr("Shots (Mouse1)"), &m_fFireSpeed, 1.0f, 32.0f, XorStr("%.1f"));
	IMGUI_TIPS("按住 开枪 时的游戏速度调整（放开会还原）。");

	ImGui::TreePop();
}

void CSpeedHacker::OnCreateMove(CUserCmd * cmd, bool *)
{
	if (m_bPositionAdjustment)
		RunPositionAdjustment(cmd);
	
	if (!m_bActive)
		return;
	
	if (m_fUseSpeed != 1.0f && (cmd->buttons & IN_USE))
		g_pSpeedModifier->SetSpeed(m_fUseSpeed);
	else if (m_fWalkSpeed != 1.0f && (cmd->buttons & IN_SPEED))
		g_pSpeedModifier->SetSpeed(m_fWalkSpeed);
	else if (m_fFireSpeed != 1.0f && (cmd->buttons & IN_ATTACK))
		g_pSpeedModifier->SetSpeed(m_fFireSpeed);
	else
		g_pSpeedModifier->SetSpeed(m_fOriginSpeed);
}

void CSpeedHacker::OnConfigLoading(const config_type & data)
{
	const std::string mainKeys = XorStr("SpeedHacks");
	
	m_bActive = g_pConfig->GetBoolean(mainKeys, XorStr("speedhack_enable"), m_bActive);
	m_bPositionAdjustment = g_pConfig->GetBoolean(mainKeys, XorStr("speedhack_posadj"), m_bPositionAdjustment);
	m_fOriginSpeed = g_pConfig->GetFloat(mainKeys, XorStr("speedhack_default"), m_fOriginSpeed);
	m_fUseSpeed = g_pConfig->GetFloat(mainKeys, XorStr("speedhack_use"), m_fUseSpeed);
	m_fWalkSpeed = g_pConfig->GetFloat(mainKeys, XorStr("speedhack_walk"), m_fWalkSpeed);
	m_fFireSpeed = g_pConfig->GetFloat(mainKeys, XorStr("speedhack_fire"), m_fFireSpeed);
}

void CSpeedHacker::OnConfigSave(config_type & data)
{
	const std::string mainKeys = XorStr("SpeedHacks");
	
	g_pConfig->SetValue(mainKeys, XorStr("speedhack_enable"), m_bActive);
	g_pConfig->SetValue(mainKeys, XorStr("speedhack_posadj"), m_bPositionAdjustment);
	g_pConfig->SetValue(mainKeys, XorStr("speedhack_default"), m_fOriginSpeed);
	g_pConfig->SetValue(mainKeys, XorStr("speedhack_use"), m_fUseSpeed);
	g_pConfig->SetValue(mainKeys, XorStr("speedhack_walk"), m_fWalkSpeed);
	g_pConfig->SetValue(mainKeys, XorStr("speedhack_fire"), m_fFireSpeed);
}

void CSpeedHacker::RunPositionAdjustment(CUserCmd * cmd)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr)
		return;

	static ConVar* cvar_cl_interp = g_pInterface->Cvar->FindVar(XorStr("cl_interp"));
	static ConVar* cvar_cl_updaterate = g_pInterface->Cvar->FindVar(XorStr("cl_updaterate"));
	static ConVar* cvar_sv_maxupdaterate = g_pInterface->Cvar->FindVar(XorStr("sv_maxupdaterate"));
	static ConVar* cvar_sv_minupdaterate = g_pInterface->Cvar->FindVar(XorStr("sv_minupdaterate"));
	static ConVar* cvar_cl_interp_ratio = g_pInterface->Cvar->FindVar(XorStr("cl_interp_ratio"));

	float cl_interp = cvar_cl_interp->GetFloat();
	int cl_updaterate = cvar_cl_updaterate->GetInt();
	int sv_maxupdaterate = cvar_sv_maxupdaterate->GetInt();
	int sv_minupdaterate = cvar_sv_minupdaterate->GetInt();
	int cl_interp_ratio = cvar_cl_interp_ratio->GetInt();

	if (sv_maxupdaterate <= cl_updaterate)
		cl_updaterate = sv_maxupdaterate;
	if (sv_minupdaterate > cl_updaterate)
		cl_updaterate = sv_minupdaterate;

	float new_interp = static_cast<float>(cl_interp_ratio / cl_updaterate);
	if (new_interp > cl_interp)
		cl_interp = new_interp;

	float simulationTime = local->GetNetProp<float>(XorStr("DT_BasePlayer"), XorStr("m_flSimulationTime"));
	// float animTime = local->GetNetProp<float>(XorStr("DT_BasePlayer"), XorStr("m_flAnimTime"));
	// int tickDifference = static_cast<int>(0.5f + (simulationTime - animTime) / g_pInterface->GlobalVars->interval_per_tick);
	int result = TIME_TO_TICKS(cl_interp) + TIME_TO_TICKS(simulationTime);
	if ((result - cmd->tick_count) >= -50)
	{
		// 优化 tick
		cmd->tick_count = result;
	}
}
