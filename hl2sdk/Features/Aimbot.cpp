#include "Aimbot.h"
#include "NoRecoilSpread.h"
#include "../Utils/math.h"
#include "../interfaces.h"
#include "../hook.h"

CAimBot* g_pAimbot = nullptr;

CAimBot::CAimBot() : CBaseFeatures::CBaseFeatures()
{
}

CAimBot::~CAimBot()
{
	CBaseFeatures::~CBaseFeatures();
}

void CAimBot::OnCreateMove(CUserCmd * cmd, bool * bSendPacket)
{
	if (!m_bActive)
	{
		m_bRunAutoAim = false;
		return;
	}

	if (m_bOnFire && !(cmd->buttons & IN_ATTACK))
	{
		m_bRunAutoAim = false;
		return;
	}

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive() || local->GetAttacker() != nullptr)
	{
		m_bRunAutoAim = false;
		return;
	}

	CBaseWeapon* weapon = local->GetActiveWeapon();
	if (!HasValidWeapon(weapon))
	{
		m_bRunAutoAim = false;
		return;
	}

	FindTarget(cmd->viewangles);
	if (m_pAimTarget == nullptr)
	{
		m_bRunAutoAim = false;
		return;
	}

	m_vecAimAngles = math::CalculateAim(local->GetEyePosition(), m_pAimTarget->GetHeadOrigin());
	// m_vecAimAngles = (m_pAimTarget->GetHeadOrigin() - local->GetEyePosition()).Normalize().toAngles();

	if (!m_vecAimAngles.IsValid())
	{
		m_bRunAutoAim = false;
		return;
	}

	if (m_bPerfectSilent)
		g_pViewManager->ApplySilentAngles(m_vecAimAngles);
	else if (m_bSilent)
		cmd->viewangles = m_vecAimAngles;
	else
		g_pInterface->Engine->SetViewAngles(m_vecAimAngles);

	m_bRunAutoAim = true;
}

void CAimBot::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("AimBot")))
		return;

	ImGui::Checkbox(XorStr("AutoAim Allow"), &m_bActive);
	ImGui::Checkbox(XorStr("AutoAim Initiative"), &m_bOnFire);
	// ImGui::Checkbox(XorStr("AutoAim No Spread"), &m_bAntiSpread);
	// ImGui::Checkbox(XorStr("AutoAim No Recoil"), &m_bAntiPunch);

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Silent Aim"), &m_bSilent);
	ImGui::Checkbox(XorStr("Perfect Silent"), &m_bPerfectSilent);
	ImGui::Checkbox(XorStr("Visible inspection"), &m_bVisible);
	ImGui::Checkbox(XorStr("Ignore allies"), &m_bNonFriendly);
	ImGui::Checkbox(XorStr("Ignore Witchs"), &m_bNonWitch);

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Distance priority"), &m_bDistance);
	ImGui::SliderFloat(XorStr("Aimbot Fov"), &m_fAimFov, 1.0f, 360.0f, XorStr("%.0f"));
	ImGui::SliderFloat(XorStr("Aimbot Distance"), &m_fAimDist, 1.0f, 5000.0f, XorStr("%.0f"));

	ImGui::Separator();
	ImGui::Checkbox(XorStr("AutoAim Range"), &m_bShowRange);
	ImGui::Checkbox(XorStr("AutoAim Angles"), &m_bShowAngles);

	ImGui::TreePop();
}

void CAimBot::OnEnginePaint(PaintMode_t mode)
{
	if (!m_bShowRange)
		return;

	int width = 0, height = 0;
	g_pInterface->Engine->GetScreenSize(width, height);
	width /= 2;
	height /= 2;

	if(m_bRunAutoAim)
		g_pDrawing->DrawCircle(width, height, m_fAimFov, CDrawing::GREEN, 8);
	else
		g_pDrawing->DrawCircle(width, height, m_fAimFov, CDrawing::WHITE, 8);
}

void CAimBot::OnFrameStageNotify(ClientFrameStage_t stage)
{
	if (!m_bShowAngles || !m_bRunAutoAim)
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr)
		return;

	CBasePlayer* hitEntity = nullptr;
	Vector eyePosition = local->GetEyePosition();
	Vector aimPosition = GetAimPosition(local, eyePosition, &hitEntity);

	g_pInterface->DebugOverlay->AddLineOverlay(eyePosition, aimPosition, 64, 128, 128, false, 0.01f);

	Vector screenPosition;
	if (hitEntity != nullptr && math::WorldToScreenEx(aimPosition, screenPosition))
		g_pDrawing->DrawText(screenPosition.x, screenPosition.y, CDrawing::PURPLE, true, "X");
}

CBasePlayer * CAimBot::FindTarget(const QAngle& myEyeAngles)
{
	m_pAimTarget = nullptr;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr)
		return nullptr;
	
	Vector myEyePosition = local->GetEyePosition();

	float minFov = 361.0f, minDistance = 65535.0f;
	int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();
	int maxClient = g_pInterface->Engine->GetMaxClients();
	for (int i = 1; i <= maxEntity; ++i)
	{
		CBasePlayer* entity = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(i));
		if (entity == local || !IsValidTarget(entity))
			continue;

		Vector aimPosition = entity->GetHeadOrigin();
		float fov = math::GetAnglesFieldOfView(myEyeAngles, math::CalculateAim(myEyePosition, aimPosition));
		float dist = math::GetVectorDistance(myEyePosition, aimPosition, true);

		if (m_bDistance)
		{
			// 距离优先
			if (fov <= m_fAimFov && dist < minDistance)
			{
				m_pAimTarget = entity;
				minDistance = dist;
			}
		}
		else
		{
			// 范围优先
			if (dist <= m_fAimDist && fov < minFov)
			{
				m_pAimTarget = entity;
				minFov = fov;
			}
		}

		if (i >= maxClient && m_pAimTarget != nullptr)
			break;
	}

	return m_pAimTarget;
}

bool CAimBot::IsTargetVisible(CBasePlayer * entity)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || entity == nullptr || !entity->IsAlive())
		return false;

	Ray_t ray;
	ray.Init(local->GetEyePosition(), entity->GetHeadOrigin());

	CTraceFilter filter;
	filter.pSkip1 = local;

	trace_t trace;

	try
	{
		g_pInterface->Trace->TraceRay(ray, MASK_SHOT, &filter, &trace);
	}
	catch (...)
	{
		Utils::log(XorStr("CAimBot.IsTargetVisible.TraceRay Error."));
		return true;
	}

	return (trace.m_pEnt == entity || trace.fraction > 0.97f);
}

bool CAimBot::IsValidTarget(CBasePlayer * entity)
{
	if (entity == nullptr || !entity->IsAlive())
		return false;

	if (m_bNonFriendly)
	{
		CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
		if (local != nullptr)
		{
			if (entity->GetTeam() == local->GetTeam())
				return false;
		}
	}

	if (m_bVisible)
	{
		if (!IsTargetVisible(entity))
			return false;
	}

	if (m_bNonWitch)
	{
		if (entity->GetClassID() == ET_WITCH)
		{
			if (entity->GetNetProp<float>(XorStr("DT_Witch"), XorStr("m_rage")) < 1.0f)
				return false;
		}
	}

	return true;
}

bool CAimBot::HasValidWeapon(CBaseWeapon * weapon)
{
	if (weapon == nullptr || weapon->GetClip() <= 0)
		return false;

	int ammoType = weapon->GetAmmoType();
	if (ammoType < AT_Pistol || ammoType > AT_Turret)
		return false;

	return (weapon->GetPrimaryAttackDelay() <= 0.0f);
}

Vector CAimBot::GetAimPosition(CBasePlayer* local, const Vector& eyePosition, CBasePlayer** hitEntity)
{
	Ray_t ray;
	CTraceFilter filter;
	ray.Init(eyePosition, m_vecAimAngles.Forward().Scale(1500.0f) + eyePosition);
	filter.pSkip1 = local;

	trace_t trace;

	try
	{
		g_pInterface->Trace->TraceRay(ray, MASK_SHOT, &filter, &trace);
	}
	catch (...)
	{
		Utils::log(XorStr("CKnifeBot.HasEnemyVisible.TraceRay Error."));
		return false;
	}

	if (hitEntity != nullptr)
		*hitEntity = reinterpret_cast<CBasePlayer*>(trace.m_pEnt);

	return trace.end;
}
