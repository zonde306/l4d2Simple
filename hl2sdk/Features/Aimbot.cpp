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
	if (!m_bActive || !(*bSendPacket))
		return;
	
	QAngle aimAngles = RunAimbot(cmd);
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

QAngle CAimBot::RunAimbot(CUserCmd * cmd)
{
	QAngle aimAngles;
	aimAngles.Invalidate();
	m_bRunning = false;
	
	if (!m_bActive)
		return aimAngles;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return aimAngles;

	CBaseWeapon* weapon = local->GetActiveWeapon();
	if (weapon == nullptr || !weapon->CanFire())
		return aimAngles;

	FindTarget(cmd->viewangles);
	if (m_pAimTarget == nullptr)
		return aimAngles;

	aimAngles = math::CalculateAim(local->GetEyePosition(), m_pAimTarget->GetHeadOrigin());
	m_bRunning = true;

	if (m_bPerfectSilent)
		return aimAngles;

	if (!m_bSilent)
		g_pClientInterface->Engine->SetViewAngles(aimAngles);

	cmd->viewangles = aimAngles;

	aimAngles.Invalidate();
	return aimAngles;
}

void CAimBot::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("AimBot")))
		return;

	ImGui::Checkbox(XorStr("AutoAim Allow"), &m_bActive);
	ImGui::Checkbox(XorStr("Trigger No Spread"), &m_bAntiSpread);
	ImGui::Checkbox(XorStr("Trigger No Recoil"), &m_bAntiPunch);

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

	ImGui::TreePop();
}

CBasePlayer * CAimBot::FindTarget(const QAngle& myEyeAngles)
{
	m_pAimTarget = nullptr;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr)
		return nullptr;
	
	Vector myEyePosition = local->GetEyePosition();
	int maxEntity = g_pClientInterface->Engine->GetMaxClients(), i = 0;
	float minFov = 361.0f, minDistance = 65535.0f;

	auto _CheckTarget = [&](int index) -> bool
	{
		CBasePlayer* entity = reinterpret_cast<CBasePlayer*>(g_pClientInterface->EntList->GetClientEntity(index));
		if (!IsValidTarget(entity))
			return false;

		Vector aimPosition = entity->GetHeadOrigin();
		float fov = math::GetAnglesFieldOfView(myEyeAngles, math::CalculateAim(myEyePosition, aimPosition));
		float dist = math::GetVectorLength(myEyePosition, aimPosition);

		// 距离太近了，可能是自己
		if (dist <= 1.0f)
			return false;

		if (m_bDistance)
		{
			// 距离优先
			if (fov <= m_fAimFov && dist < minDistance)
			{
				m_pAimTarget = entity;
				minDistance = dist;
				return true;
			}
		}
		else
		{
			// 范围优先
			if (dist <= m_fAimDist && fov < minFov)
			{
				m_pAimTarget = entity;
				minFov = fov;
				return true;
			}
		}

		return false;
	};

	for (i = 1; i <= maxEntity; ++i)
	{
		// 检查玩家敌人
		_CheckTarget(i);
	}

	if (m_pAimTarget != nullptr || local->GetTeam() != 2)
		return m_pAimTarget;

	i = maxEntity + 1;
	maxEntity = g_pClientInterface->EntList->GetHighestEntityIndex();
	for (; i <= maxEntity; ++i)
	{
		// 检查普感敌人
		_CheckTarget(i);
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
		g_pClientInterface->Trace->TraceRay(ray, MASK_SHOT, &filter, &trace);
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
