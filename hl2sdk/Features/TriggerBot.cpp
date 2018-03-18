#include "TriggerBot.h"
#include "NoRecoilSpread.h"
#include "../interfaces.h"
#include "../hook.h"

#define HITBOX_COMMON			15	// 普感
#define HITBOX_PLAYER			10	// 生还者/特感
#define HITBOX_COMMON_1			14
#define HITBOX_COMMON_2			15
#define HITBOX_COMMON_3			16
#define HITBOX_COMMON_4			17
#define HITBOX_JOCKEY			4
#define HITBOX_SPITTER			4
#define HITBOX_CHARGER			9
#define HITBOX_WITCH			10

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
	if (m_pAimTarget == nullptr)
		return aimAngles;

	// 开枪
	cmd->buttons |= IN_ATTACK;
	m_bRunning = true;

	if (m_bFollowEnemy)
	{
		aimAngles = CalculateAim(player->GetEyePosition(), m_pAimTarget->GetHitboxOrigin(m_iHitBox));
		if (GetAnglesFieldOfView(cmd->viewangles, aimAngles) <= m_fFollowFov)
		{
			g_pClientInterface->Engine->SetViewAngles(aimAngles);
		}
	}

	if (m_bTraceHead)
	{
		aimAngles = CalculateAim(player->GetEyePosition(), GetHeadPosition(m_pAimTarget));
		if (GetAnglesFieldOfView(cmd->viewangles, aimAngles) <= m_fTraceFov)
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

Vector CTriggerBot::GetHeadPosition(CBasePlayer * entity)
{
	if (!IsValidTarget(entity))
		return Vector();

	int classId = entity->GetClassID();
	if (classId == ET_SURVIVORBOT || classId == ET_CTERRORPLAYER || classId == ET_TANK ||
		classId == ET_WITCH || classId == ET_SMOKER || classId == ET_BOOMER || classId == ET_HUNTER)
		return entity->GetHitboxOrigin(HITBOX_PLAYER);

	if(classId == ET_JOCKEY || classId == ET_SPITTER)
		return entity->GetHitboxOrigin(HITBOX_JOCKEY);

	if(classId == ET_CHARGER)
		return entity->GetHitboxOrigin(HITBOX_CHARGER);

	if (classId == ET_INFECTED)
		return entity->GetHitboxOrigin(HITBOX_COMMON);

	return Vector();
}
