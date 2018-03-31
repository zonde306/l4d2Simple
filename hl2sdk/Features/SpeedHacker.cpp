#include "SpeedHacker.h"
#include "../../l4d2Simple2/speedhack.h"

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
