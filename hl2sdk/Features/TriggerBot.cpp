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
	if (!m_bActive)
		return;

	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive() || player->GetAttacker() != nullptr)
		return;

	CBaseWeapon* weapon = player->GetActiveWeapon();
	if (!HasValidWeapon(weapon))
		return;

	GetAimTarget(cmd->viewangles);
	if (m_pAimTarget == nullptr || m_pAimTarget->GetTeam() == player->GetTeam())
		return;

	// 开枪
	cmd->buttons |= IN_ATTACK;

	if (m_bFollowEnemy)
	{
		QAngle aimAngles = math::CalculateAim(player->GetEyePosition(), m_pAimTarget->GetHitboxOrigin(m_iHitBox));
		if (math::GetAnglesFieldOfView(cmd->viewangles, aimAngles) <= m_fFollowFov)
		{
			g_pInterface->Engine->SetViewAngles(aimAngles);
		}
	}

	if (m_bTraceHead)
	{
		QAngle aimAngles = math::CalculateAim(player->GetEyePosition(), m_pAimTarget->GetHeadOrigin());
		if (math::GetAnglesFieldOfView(cmd->viewangles, aimAngles) <= m_fTraceFov)
		{
			g_pViewManager->ApplySilentAngles(aimAngles);
		}
	}
}

void CTriggerBot::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("TriggerBot")))
		return;

	ImGui::Checkbox(XorStr("Trigger Allow"), &m_bActive);
	ImGui::Checkbox(XorStr("Trigger Crosshairs"), &m_bCrosshairs);
	// ImGui::Checkbox(XorStr("Trigger No Spread"), &m_bAntiSpread);
	// ImGui::Checkbox(XorStr("Trigger No Recoil"), &m_bAntiPunch);

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Track head"), &m_bTraceHead);
	ImGui::Checkbox(XorStr("Track Silent"), &m_bTraceSilent);
	ImGui::SliderFloat(XorStr("Track FOV"), &m_fTraceFov, 1.0f, 90.0f, ("%.1f"));

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Follow the target"), &m_bFollowEnemy);
	ImGui::SliderFloat(XorStr("Follow FOV"), &m_fFollowFov, 1.0f, 90.0f, ("%.1f"));

	ImGui::TreePop();
}

void CTriggerBot::OnEnginePaint(PaintMode_t mode)
{
	if (!m_bCrosshairs)
		return;

	D3DCOLOR color = CDrawing::WHITE;

	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr)
		return;

	if (m_pAimTarget == nullptr)
	{
		color = CDrawing::LAWNGREEN;
		// g_pInterface->Surface->DrawSetColor(128, 255, 0, 255);
	}
	else if (player->GetTeam() == m_pAimTarget->GetTeam())
	{
		color = CDrawing::SKYBLUE;
		// g_pInterface->Surface->DrawSetColor(0, 255, 255, 255);
	}
	else
	{
		color = CDrawing::RED;
		// g_pInterface->Surface->DrawSetColor(255, 0, 0, 255);
	}

	int width, height;
	g_pInterface->Engine->GetScreenSize(width, height);
	width /= 2;
	height /= 2;

	// g_pInterface->Surface->DrawLine(width - 5, height, width + 5, height);
	// g_pInterface->Surface->DrawLine(width, height - 5, width, height + 5);
	g_pDrawing->DrawLine(width - 10, height, width + 10, height, color);
	g_pDrawing->DrawLine(width, height - 10, width, height + 10, color);
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
		g_pInterface->Trace->TraceRay(ray, MASK_SHOT, &filter, &trace);
	}
	catch (...)
	{
		Utils::log(XorStr("CTriggerBot.GetAimTarget.TraceRay Error."));
		m_pAimTarget = nullptr;
		return nullptr;
	}

	if (trace.m_pEnt == player || !IsValidTarget(reinterpret_cast<CBasePlayer*>(trace.m_pEnt)))
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

bool CTriggerBot::HasValidWeapon(CBaseWeapon * weapon)
{
	if (weapon == nullptr || weapon->GetClip() <= 0)
		return false;

	int ammoType = weapon->GetAmmoType();
	if (ammoType < AT_Pistol || ammoType > AT_Turret)
		return false;

	return true;
}
