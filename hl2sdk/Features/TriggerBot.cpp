#include "TriggerBot.h"
#include "NoRecoilSpread.h"
#include "../Utils/math.h"
#include "../interfaces.h"
#include "../hook.h"

CTriggerBot* g_pTriggerBot = nullptr;

CTriggerBot::CTriggerBot() : CBaseFeatures::CBaseFeatures()
{
}

CTriggerBot::~CTriggerBot()
{
	CBaseFeatures::~CBaseFeatures();
}

void CTriggerBot::OnCreateMove(CUserCmd * cmd, bool * bSendPacket)
{
	if (!m_bActive || !(*bSendPacket))
		return;
	
	QAngle aimAngles = RunTrigger(cmd);
	if (aimAngles.IsValid())
	{
		*bSendPacket = false;
		g_pViewAnglesManager->StartSilent(cmd);
		cmd->viewangles = aimAngles;
	}
	else
	{
		g_pViewAnglesManager->FinishSilent(cmd);
	}

	if (m_bRunning)
	{
		if (m_bAntiSpread)
			g_pViewAnglesManager->RemoveSpread(cmd);

		if (m_bAntiPunch)
			g_pViewAnglesManager->RemoveRecoil(cmd);
	}
}

QAngle CTriggerBot::RunTrigger(CUserCmd * cmd)
{
	QAngle aimAngles;
	aimAngles.Invalidate();
	m_bRunning = false;

	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive())
		return aimAngles;

	CBaseWeapon* weapon = player->GetActiveWeapon();
	if (weapon == nullptr || !weapon->CanFire())
		return aimAngles;

	GetAimTarget(cmd->viewangles);
	if (m_pAimTarget == nullptr || m_pAimTarget->GetTeam() == player->GetTeam())
		return aimAngles;

	// 开枪
	cmd->buttons |= IN_ATTACK;
	m_bRunning = true;

	if (m_bFollowEnemy)
	{
		aimAngles = math::CalculateAim(player->GetEyePosition(), m_pAimTarget->GetHitboxOrigin(m_iHitBox));
		if (math::GetAnglesFieldOfView(cmd->viewangles, aimAngles) <= m_fFollowFov)
		{
			g_pClientInterface->Engine->SetViewAngles(aimAngles);
		}
	}

	if (m_bTraceHead)
	{
		aimAngles = math::CalculateAim(player->GetEyePosition(), m_pAimTarget->GetHeadOrigin());
		if (math::GetAnglesFieldOfView(cmd->viewangles, aimAngles) <= m_fTraceFov)
		{
			return aimAngles;
		}
	}

	aimAngles.Invalidate();
	return aimAngles;
}

void CTriggerBot::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("TriggerBot")))
		return;

	ImGui::Checkbox(XorStr("Trigger Allow"), &m_bActive);
	ImGui::Checkbox(XorStr("Trigger Crosshairs"), &m_bCrosshairs);
	ImGui::Checkbox(XorStr("Trigger No Spread"), &m_bAntiSpread);
	ImGui::Checkbox(XorStr("Trigger No Recoil"), &m_bAntiPunch);

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Track head"), &m_bTraceHead);
	ImGui::Checkbox(XorStr("Track Silent"), &m_bTraceSilent);
	ImGui::SliderFloat(XorStr("Track FOV"), &m_fTraceFov, 1.0f, 90.0f, ("%.1f"));

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Follow the target"), &m_bFollowEnemy);
	ImGui::SliderFloat(XorStr("Follow FOV"), &m_fFollowFov, 1.0f, 90.0f, ("%.1f"));

	ImGui::TreePop();
}

void CTriggerBot::OnPaintTraverse(VPANEL panel)
{
	static VPANEL FocusOverlayPanel = 0;
	if (FocusOverlayPanel == 0)
	{
		const char* panelName = g_pClientInterface->Panel->GetName(panel);
		if (panelName[0] == 'F' && panelName[5] == 'O')
			FocusOverlayPanel = panel;
	}

	if (panel != FocusOverlayPanel || !m_bCrosshairs)
		return;

	static bool ignoreFrame = false;
	if ((ignoreFrame = !ignoreFrame))
		return;

	D3DCOLOR color = CDrawing::WHITE;

	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr)
		return;

	if (m_pAimTarget == nullptr)
		color = CDrawing::LAWNGREEN;
	else if (player->GetTeam() == m_pAimTarget->GetTeam())
		color = CDrawing::SKYBLUE;
	else
		color = CDrawing::RED;

	int width, height;
	g_pClientInterface->Engine->GetScreenSize(width, height);
	g_pDrawing->DrawLine(width - 5, height, width + 5, height, color);
	g_pDrawing->DrawLine(width, height - 5, width, height + 5, color);
}

CBasePlayer * CTriggerBot::GetAimTarget(const QAngle& eyeAngles)
{
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	
	CTraceFilter filter;
	filter.pSkip1 = player;

	Ray_t ray;
	Vector startPosition = player->GetEyePosition();
	Vector endPosition = eyeAngles.Forward().Scale(3500.0f) + startPosition;
	ray.Init(startPosition, endPosition);

	trace_t trace;

	try
	{
		g_pClientInterface->Trace->TraceRay(ray, MASK_SHOT, &filter, &trace);
	}
	catch (...)
	{
		Utils::log(XorStr("CTriggerBot.GetAimTarget.TraceRay Error."));
		m_pAimTarget = nullptr;
		return nullptr;
	}

	if (!IsValidTarget(reinterpret_cast<CBasePlayer*>(trace.m_pEnt)))
	{
		m_pAimTarget = nullptr;
		return nullptr;
	}

	m_pAimTarget = reinterpret_cast<CBasePlayer*>(trace.m_pEnt);
	m_iHitBox = trace.hitbox;
	m_iHitGroup = trace.hitGroup;

	return m_pAimTarget;
}

bool CTriggerBot::IsValidTarget(CBasePlayer * entity)
{
	if (entity == nullptr || !entity->IsAlive())
		return false;

	return true;
}
