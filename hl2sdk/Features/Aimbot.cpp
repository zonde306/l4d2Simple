﻿#include "Aimbot.h"
#include "NoRecoilSpread.h"
#include "../Utils/math.h"
#include "../interfaces.h"
#include "../hook.h"
#include "../../l4d2Simple2/config.h"
#include <cmath>

CAimBot* g_pAimbot = nullptr;

#ifndef M_PI
#define M_PI	3.14159265358979323846
#define M_PI_F	((float)M_PI)
#endif

#ifndef RAD2DEG
#define RAD2DEG(x)  ((float)(x) * (float)(180.f / M_PI_F))
#define RadiansToDegrees RAD2DEG
#define DEG2RAD(x)  ((float)(x) * (float)(M_PI_F / 180.f))
#define DegreesToRadians DEG2RAD
#endif

#define IsSubMachinegun(_id)		(_id == WeaponId_SubMachinegun || _id == WeaponId_Silenced || _id == WeaponId_MP5)
#define IsShotgun(_id)				(_id == WeaponId_PumpShotgun || _id == WeaponId_Chrome || _id == WeaponId_AutoShotgun || _id == WeaponId_SPAS)
#define IsAssaultRifle(_id)			(_id == WeaponId_AssaultRifle || _id == WeaponId_AK47 || _id == WeaponId_Desert || _id == WeaponId_SG552 || _id == WeaponId_M60)
#define IsSniper(_id)				(_id == WeaponId_SniperRifle || _id == WeaponId_Military || _id == WeaponId_Scout || _id == WeaponId_AWP)
#define IsPistol(_id)				(_id == WeaponId_Pistol || _id == WeaponId_MagnumPistol)
#define IsMedical(_id)				(_id == WeaponId_FirstAidKit || _id == WeaponId_ItemDefibrillator || _id == WeaponId_PainPills || _id == WeaponId_Adrenaline)
#define IsAmmoPack(_id)				(_id == WeaponId_ItemAmmoPack || _id == WeaponId_ItemUpgradePackExplosive || _id == WeaponId_ItemUpgradePackIncendiary)
#define IsMelee(_id)				(_id == WeaponId_TerrorMeleeWeapon || _id == WeaponId_Chainsaw)
#define IsWeaponT1(_id)				(IsSubMachinegun(_id) || _id == WeaponId_PumpShotgun || _id == WeaponId_Chrome || _id == WeaponId_Pistol)
#define IsWeaponT2(_id)				(_id == WeaponId_AutoShotgun || _id == WeaponId_SPAS || _id == WeaponId_AssaultRifle || _id == WeaponId_AK47 || _id == WeaponId_Desert || _id == WeaponId_SG552 || _id == WeaponId_MagnumPistol || IsSniper(_id))
#define IsWeaponT3(_id)				(_id == WeaponId_M60 || _id == WeaponId_GrenadeLauncher)
#define IsNotGunWeapon(_id)			(IsGrenadeWeapon(_id) || IsMedicalWeapon(_id) || IsPillsWeapon(_id) || IsCarryWeapon(_id) || _id == Weapon_Melee || _id == Weapon_Chainsaw)
#define IsGunWeapon(_id)			(IsSubMachinegun(_id) || IsShotgun(_id) || IsAssaultRifle(_id) || IsSniper(_id) || IsPistol(_id))
#define IsGrenadeWeapon(_id)		(_id == Weapon_Molotov || _id == Weapon_PipeBomb || _id == Weapon_Vomitjar)
#define IsMedicalWeapon(_id)		(_id == Weapon_FirstAidKit || _id == Weapon_Defibrillator || _id == Weapon_FireAmmo || _id == Weapon_ExplodeAmmo)
#define IsPillsWeapon(_id)			(_id == Weapon_PainPills || _id == Weapon_Adrenaline)
#define IsCarryWeapon(_id)			(_id == Weapon_Gascan || _id == Weapon_Fireworkcrate || _id == Weapon_Propanetank || _id == Weapon_Oxygentank || _id == Weapon_Gnome || _id == Weapon_Cola)
#define IsSpecialInfected(_id)		(_id == ET_BOOMER || _id == ET_HUNTER || _id == ET_SMOKER || _id == ET_SPITTER || _id == ET_JOCKEY || _id == ET_CHARGER || _id == ET_TANK)

#define ANIM_CHARGER_CHARGING		5
#define ANIM_SMOKER_PULLING			30
#define ANIM_SMOKER_SHOOTING		27
#define ANIM_HUNTER_LUNGING			67
#define ANIM_JOCKEY_LEAPING			10
#define ANIM_JOCKEY_RIDEING			8

CAimBot::CAimBot() : CBaseFeatures::CBaseFeatures()
{
}

CAimBot::~CAimBot()
{
	CBaseFeatures::~CBaseFeatures();
}

void CAimBot::OnCreateMove(CUserCmd * cmd, bool * bSendPacket)
{
	if (!m_bActive || m_bMenuOpen || (cmd->buttons & IN_SCORE))
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
	if (!CanRunAimbot(local))
	{
		m_bRunAutoAim = false;
		return;
	}

	QAngle viewAngles;
	g_pInterface->Engine->GetViewAngles(viewAngles);
	Vector aimHeadOrigin = INVALID_VECTOR;
	if (!m_bRemeberChoose || !IsValidTarget(m_pAimTarget, true))
		m_pAimTarget = FindTarget(viewAngles, &aimHeadOrigin);
	else
		aimHeadOrigin = GetTargetAimPosition(m_pAimTarget);

	if (!aimHeadOrigin.IsValid() || m_pAimTarget == nullptr)
	{
		// Utils::logError(XorStr("Aimbot Can't get target location"));
		m_pAimTarget = nullptr;
		m_bRunAutoAim = false;
		return;
	}

	Vector myEyeOrigin = local->GetEyePosition();
	if (m_bVelExt)
	{
		myEyeOrigin = math::VelocityExtrapolate(myEyeOrigin, local->GetVelocity(), m_bForwardtrack);
		aimHeadOrigin = math::VelocityExtrapolate(aimHeadOrigin, m_pAimTarget->GetVelocity(), m_bForwardtrack);
	}

	m_vecAimAngles = math::CalculateAim(myEyeOrigin, aimHeadOrigin);
	// m_vecAimAngles = (m_pAimTarget->GetHeadOrigin() - local->GetEyePosition()).Normalize().toAngles();

	if (!m_vecAimAngles.IsValid())
	{
		m_bRunAutoAim = false;
		return;
	}

	if (m_bPerfectSilent)
		g_pViewManager->ApplySilentFire(m_vecAimAngles);
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
	IMGUI_TIPS("自动瞄准。");

	ImGui::Checkbox(XorStr("AutoAim Initiative"), &m_bOnFire);
	IMGUI_TIPS("开枪时才会自动瞄准。");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Silent Aim"), &m_bSilent);
	IMGUI_TIPS("自己看不到自动瞄准，建议开启。");

	ImGui::Checkbox(XorStr("Perfect Silent"), &m_bPerfectSilent);
	IMGUI_TIPS("观察者看不到自动瞄准，建议开启。");

	ImGui::Checkbox(XorStr("Visible inspection"), &m_bVisible);
	IMGUI_TIPS("可见检查，看不见的敌人不瞄准。");

	ImGui::Checkbox(XorStr("Ignore allies"), &m_bNonFriendly);
	IMGUI_TIPS("自动瞄准不瞄准队友。");

	ImGui::Checkbox(XorStr("Ignore Witchs"), &m_bNonWitch);
	IMGUI_TIPS("自动瞄准不瞄准 Witch。");
	
	ImGui::Checkbox(XorStr("Ignore Tank"), &m_bIgnoreTank);
	IMGUI_TIPS("自动瞄准不瞄准 Tank。");
	
	ImGui::Checkbox(XorStr("Ignore Common Infected"), &m_bIgnoreCI);
	IMGUI_TIPS("自动瞄准不瞄准普感。");
	
	ImGui::Checkbox(XorStr("Shotgun Chest"), &m_bShotgunChest);
	IMGUI_TIPS("霰弹枪瞄准身体");
	
	ImGui::Checkbox(XorStr("Fatal First"), &m_bFatalFirst);
	IMGUI_TIPS("优先选择致命（危险）目标");
	
	ImGui::Checkbox(XorStr("Ignore Boomer"), &m_bIgnoreNearSurvivor);
	IMGUI_TIPS("忽略接近生还者的 Boomer");
	
	ImGui::Checkbox(XorStr("Melee Aimbot"), &m_bAimOfMelee);
	IMGUI_TIPS("允许使用近战武器进行瞄准");
	
	ImGui::Checkbox(XorStr("Remember Target"), &m_bRemeberChoose);
	IMGUI_TIPS("连续攻击直至目标死亡");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Velocity Extrapolate"), &m_bVelExt);
	IMGUI_TIPS("速度预测，开启以提高精度。");

	ImGui::Checkbox(XorStr("Forwardtrack"), &m_bForwardtrack);
	IMGUI_TIPS("速度延迟预测，开启以提高精度，需要先开启上面那个才会运行。");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Distance priority"), &m_bDistance);
	IMGUI_TIPS("优先选择最接距离近的目标，如果不开启则优先选择最接近准星的目标。");

	ImGui::SliderFloat(XorStr("Aimbot Fov"), &m_fAimFov, 1.0f, 360.0f, XorStr("%.0f"));
	IMGUI_TIPS("自动瞄准范围（半径，从准星位置开始）");

	ImGui::SliderFloat(XorStr("Aimbot Distance"), &m_fAimDist, 1.0f, 5000.0f, XorStr("%.0f"));
	IMGUI_TIPS("自动瞄准范围（距离）");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("AutoAim Range"), &m_bShowRange);
	IMGUI_TIPS("自动瞄准范围。");

	ImGui::Checkbox(XorStr("AutoAim Angles"), &m_bShowAngles);
	IMGUI_TIPS("显示瞄准的位置。");
	
	ImGui::Checkbox(XorStr("AutoAim Target"), &m_bShowTarget);
	IMGUI_TIPS("显示瞄准的目标。");
	
	ImGui::Checkbox(XorStr("Debug"), &m_bDebug);
	IMGUI_TIPS("显示调试信息（目标分数）。");

	ImGui::TreePop();
}

void CAimBot::OnConfigLoading(CProfile& cfg)
{
	const std::string mainKeys = XorStr("Aimbot");
	
	m_bActive = cfg.GetBoolean(mainKeys, XorStr("autoaim_enable"), m_bActive);
	m_bOnFire = cfg.GetBoolean(mainKeys, XorStr("autoaim_only_shot"), m_bOnFire);
	m_bSilent = cfg.GetBoolean(mainKeys, XorStr("autoaim_silent"), m_bSilent);
	m_bPerfectSilent = cfg.GetBoolean(mainKeys, XorStr("autoaim_perfect_silent"), m_bPerfectSilent);
	m_bVisible = cfg.GetBoolean(mainKeys, XorStr("autoaim_visible"), m_bVisible);
	m_bNonFriendly = cfg.GetBoolean(mainKeys, XorStr("autoaim_non_friendly"), m_bNonFriendly);
	m_bNonWitch = cfg.GetBoolean(mainKeys, XorStr("autoaim_non_witch"), m_bNonWitch);
	m_bDistance = cfg.GetBoolean(mainKeys, XorStr("autoaim_distance_priority"), m_bDistance);
	m_fAimFov = cfg.GetFloat(mainKeys, XorStr("autoaim_fov"), m_fAimFov);
	m_fAimDist = cfg.GetFloat(mainKeys, XorStr("autoaim_distance"), m_fAimDist);
	m_bShowRange = cfg.GetBoolean(mainKeys, XorStr("autoaim_show_range"), m_bShowRange);
	m_bShowAngles = cfg.GetBoolean(mainKeys, XorStr("autoaim_show_angles"), m_bShowAngles);
	m_bVelExt = cfg.GetBoolean(mainKeys, XorStr("autoaim_velext"), m_bVelExt);
	m_bForwardtrack = cfg.GetBoolean(mainKeys, XorStr("autoaim_forwardtrack"), m_bForwardtrack);
	m_bShotgunChest = cfg.GetBoolean(mainKeys, XorStr("autoaim_shotgun_chest"), m_bShotgunChest);
	m_bFatalFirst = cfg.GetBoolean(mainKeys, XorStr("autoaim_fatal_first"), m_bFatalFirst);
	m_bShowTarget = cfg.GetBoolean(mainKeys, XorStr("autoaim_target"), m_bShowTarget);
	m_bIgnoreTank = cfg.GetBoolean(mainKeys, XorStr("autoaim_non_tank"), m_bIgnoreTank);
	m_bIgnoreCI = cfg.GetBoolean(mainKeys, XorStr("autoaim_non_ci"), m_bIgnoreCI);
	m_bIgnoreCI = cfg.GetBoolean(mainKeys, XorStr("autoaim_non_near"), m_bIgnoreNearSurvivor);
	m_bIgnoreCI = cfg.GetBoolean(mainKeys, XorStr("autoaim_with_melee"), m_bAimOfMelee);
	m_bIgnoreCI = cfg.GetBoolean(mainKeys, XorStr("autoaim_remember"), m_bRemeberChoose);
}

void CAimBot::OnConfigSave(CProfile& cfg)
{
	const std::string mainKeys = XorStr("Aimbot");
	
	cfg.SetValue(mainKeys, XorStr("autoaim_enable"), m_bActive);
	cfg.SetValue(mainKeys, XorStr("autoaim_only_shot"), m_bOnFire);
	cfg.SetValue(mainKeys, XorStr("autoaim_silent"), m_bSilent);
	cfg.SetValue(mainKeys, XorStr("autoaim_perfect_silent"), m_bPerfectSilent);
	cfg.SetValue(mainKeys, XorStr("autoaim_visible"), m_bVisible);
	cfg.SetValue(mainKeys, XorStr("autoaim_non_friendly"), m_bNonFriendly);
	cfg.SetValue(mainKeys, XorStr("autoaim_non_witch"), m_bNonWitch);
	cfg.SetValue(mainKeys, XorStr("autoaim_distance_priority"), m_bDistance);
	cfg.SetValue(mainKeys, XorStr("autoaim_fov"), m_fAimFov);
	cfg.SetValue(mainKeys, XorStr("autoaim_distance"), m_fAimDist);
	cfg.SetValue(mainKeys, XorStr("autoaim_show_range"), m_bShowRange);
	cfg.SetValue(mainKeys, XorStr("autoaim_show_angles"), m_bShowAngles);
	cfg.SetValue(mainKeys, XorStr("autoaim_velext"), m_bVelExt);
	cfg.SetValue(mainKeys, XorStr("autoaim_forwardtrack"), m_bForwardtrack);
	cfg.SetValue(mainKeys, XorStr("autoaim_shotgun_chest"), m_bShotgunChest);
	cfg.SetValue(mainKeys, XorStr("autoaim_fatal_first"), m_bFatalFirst);
	cfg.SetValue(mainKeys, XorStr("autoaim_target"), m_bShowTarget);
	cfg.SetValue(mainKeys, XorStr("autoaim_non_tank"), m_bIgnoreTank);
	cfg.SetValue(mainKeys, XorStr("autoaim_non_ci"), m_bIgnoreCI);
	cfg.SetValue(mainKeys, XorStr("autoaim_non_near"), m_bIgnoreNearSurvivor);
	cfg.SetValue(mainKeys, XorStr("autoaim_with_melee"), m_bAimOfMelee);
	cfg.SetValue(mainKeys, XorStr("autoaim_remember"), m_bRemeberChoose);
}

void CAimBot::OnDisconnect()
{
	m_pAimTarget = nullptr;
}

void CAimBot::OnEnginePaint(PaintMode_t mode)
{
	if (!m_bActive)
		return;
	
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return;

	int width = 0, height = 0;
	g_pInterface->Engine->GetScreenSize(width, height);

	if (m_bShowRange)
	{
		int myFov = local->GetNetProp<byte>(XorStr("DT_BasePlayer"), XorStr("m_iFOV"));
		if (myFov == 0)
		{
			static ConVar* cl_fov = g_pInterface->Cvar->FindVar(XorStr("cl_fov"));
			if (cl_fov != nullptr)
				myFov = cl_fov->GetInt();
			else
				myFov = 90;
			// myFov = local->GetNetProp<byte>(XorStr("DT_BasePlayer"), XorStr("m_iDefaultFOV"));
		}

		float radius = (std::tanf(DEG2RAD(m_fAimFov) / 2) / std::tanf(DEG2RAD(myFov) / 2) * width);

		width /= 2;
		height /= 2;

		if (m_bRunAutoAim)
			g_pDrawing->DrawCircle(width, height, static_cast<int>(radius), CDrawing::GREEN);
		else
			g_pDrawing->DrawCircle(width, height, static_cast<int>(radius), CDrawing::WHITE);

#ifdef _DEBUG
		g_pDrawing->DrawText(width, height + 16, CDrawing::ORANGE, true, XorStr("aimDistance = %.0f"), m_fTargetDistance);
		g_pDrawing->DrawText(width, height + 32, CDrawing::YELLOW, true, XorStr("aimFov = %.0f"), m_fTargetFov);
		g_pDrawing->DrawText(width, height + 48, CDrawing::PURPLE, true, XorStr("IsShotgun = %d"), HasShotgun(local->GetActiveWeapon()));
		g_pDrawing->DrawText(width, height + 64, CDrawing::PURPLE, true, XorStr("entindex = %d"), m_iEntityIndex);
#endif
	}

	if (m_bShowTarget)
	{
		QAngle viewAngles;
		g_pInterface->Engine->GetViewAngles(viewAngles);
		Vector aimHeadOrigin = INVALID_VECTOR;
		if (!m_bRemeberChoose || !IsValidTarget(m_pAimTarget, true))
			m_pAimTarget = FindTarget(viewAngles, &aimHeadOrigin);
		else
			aimHeadOrigin = GetTargetAimPosition(m_pAimTarget);

		Vector screen;
		if (m_pAimTarget != nullptr && aimHeadOrigin.IsValid() && math::WorldToScreenEx(aimHeadOrigin, screen))
		{
			if(m_bRunAutoAim)
				g_pDrawing->DrawCircleFilled(static_cast<int>(screen.x), static_cast<int>(screen.y), 5, CDrawing::CYAN, 8);
			else
				g_pDrawing->DrawCircle(static_cast<int>(screen.x), static_cast<int>(screen.y), 5, CDrawing::CYAN, 8);

#ifdef _DEBUG
			g_pDrawing->DrawText(screen.x, screen.y - 16, CDrawing::CYAN, true, XorStr("%s(%d)"), m_pAimTarget->GetClassname(), m_pAimTarget->GetHealth());
#endif
		}
	}
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
	Vector aimPosition = GetAimbotAimPosition(local, eyePosition, &hitEntity);
	if (!aimPosition.IsValid())
		return;

	g_pInterface->DebugOverlay->AddLineOverlay(eyePosition, aimPosition, 64, 128, 128, true, 0.1f);

	Vector screenPosition;
	if (hitEntity != nullptr && math::WorldToScreenEx(aimPosition, screenPosition))
		g_pDrawing->DrawText(static_cast<int>(screenPosition.x), static_cast<int>(screenPosition.y),
			CDrawing::PURPLE, true, "X");
}

void CAimBot::OnEntityDeleted(CBaseEntity* entity)
{
	if (m_pAimTarget == entity)
		m_pAimTarget = nullptr;
}

CBasePlayer * CAimBot::FindTarget(const QAngle& myEyeAngles, Vector* aimPos)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsValid())
		return nullptr;
	
	CBasePlayer* target = nullptr;

	/*
	target = GetAimTarget(local, myEyeAngles);
	if (target && target->IsValid())
	{
		int classId = target->GetClassID();
		if (classId == ET_WeaponGascan || classId == ET_WeaponPropaneTank || classId == ET_WeaponFirework || classId == ET_WeaponOxygen || classId == ET_TankRock)
			return target;

		if (IsValidTarget(target) && IsFatalTarget(target))
			return target;
	}
	*/

	Vector aimPosition, bestAimPosition;
	Vector myEyePosition = local->GetEyePosition();

	float minFov = m_fAimFov + 0.1f, minDistance = m_fAimDist + 0.1f;
	int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();

	// 特感和普感是同一个阵营的
	if (local->GetTeam() == 3)
		maxEntity = 64;

	for (int i = 1; i <= maxEntity; ++i)
	{
		CBasePlayer* entity = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(i));
		if (entity == local || !IsValidTarget(entity, false))
			continue;

		aimPosition = GetTargetAimPosition(entity);
		if (!aimPosition.IsValid())
			continue;

		float fov = math::GetAnglesFieldOfView(myEyeAngles, math::CalculateAim(myEyePosition, aimPosition));
		float dist = math::GetVectorDistance(myEyePosition, aimPosition, true);

		if (m_bFatalFirst && i <= 64 && fov <= m_fAimFov && dist <= m_fAimDist && IsFatalTarget(entity))
		{
			m_iEntityIndex = i;
			target = entity;
			minDistance = dist;
			minFov = fov;
			bestAimPosition = aimPosition;
			break;
		}

		if (m_bDistance)
		{
			// 距离优先
			if (fov <= m_fAimFov && dist < minDistance)
			{
				m_iEntityIndex = i;
				target = entity;
				minDistance = dist;
				minFov = fov;
				bestAimPosition = aimPosition;
			}
		}
		else
		{
			// 最近优先
			if (dist <= m_fAimDist && fov < minFov)
			{
				m_iEntityIndex = i;
				target = entity;
				minDistance = dist;
				minFov = fov;
				bestAimPosition = aimPosition;
			}
		}

		// 如果已经选择了玩家敌人，则不需要再去选择普感敌人
		if (i > 64 && target != nullptr)
			break;
	}

	if (aimPos)
		*aimPos = bestAimPosition;

	m_fTargetFov = minFov;
	m_fTargetDistance = minDistance;
	return target;
}

class CTriggerTraceFilter : public CTraceFilter
{
public:
	virtual bool ShouldHitEntity(CBaseEntity* pEntityHandle, int contentsMask) override
	{
		if (pEntityHandle == pSkip1)
			return false;

		int classId = -1;

		try
		{
			classId = pEntityHandle->GetClassID();
		}
		catch (...)
		{
			return true;
		}

		if (classId == ET_SurvivorRescue)
			return false;

		if (classId == ET_CTERRORPLAYER && reinterpret_cast<CBasePlayer*>(pEntityHandle)->IsGhost())
			return false;

		return true;
	}
};

CBasePlayer* CAimBot::GetAimTarget(CBasePlayer* player, const QAngle& viewAngles)
{
	// QAngle viewAngles;
	// g_pInterface->Engine->GetViewAngles(viewAngles);
	
	CTriggerTraceFilter filter;
	filter.pSkip1 = player;

	Ray_t ray;
	Vector startPosition = player->GetEyePosition();
	if (m_bVelExt)
		startPosition = math::VelocityExtrapolate(startPosition, player->GetVelocity(), m_bForwardtrack);

	Vector endPosition = startPosition + viewAngles.Forward().Scale(3500.0f);
	ray.Init(startPosition, endPosition);

	trace_t trace;

	try
	{
		// g_pInterface->Trace->TraceRay(ray, MASK_SHOT, &filter, &trace);
		// g_pClientHook->TraceLine2(startPosition, endPosition, MASK_SHOT, player, CG_PLAYER|CG_NPC|CG_DEBRIS|CG_VEHICLE, &trace);
		g_pInterface->TraceLine(startPosition, endPosition, MASK_SHOT, &filter, &trace);
	}
	catch (...)
	{
		Utils::log(XorStr("CAimBot.GetAimTarget.TraceRay Error."));
		m_pAimTarget = nullptr;
		return nullptr;
	}

	if (trace.m_pEnt == player || !trace.m_pEnt->IsValid())
		return nullptr;

	return reinterpret_cast<CBasePlayer*>(trace.m_pEnt);
}

bool CAimBot::IsTargetVisible(CBasePlayer * entity, Vector aimPosition, unsigned int mask)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || entity == nullptr || !entity->IsAlive())
		return false;

	if (!aimPosition.IsValid())
	{
		if (m_bShotgunChest && HasShotgun(local->GetActiveWeapon()))
			aimPosition = entity->GetChestOrigin();
		else
			aimPosition = entity->GetHeadOrigin();
	}

	Ray_t ray;
	ray.Init(local->GetEyePosition(), aimPosition);

	CTriggerTraceFilter filter;
	filter.pSkip1 = local;

	trace_t trace;

	try
	{
		g_pInterface->Trace->TraceRay(ray, mask, &filter, &trace);
	}
	catch (...)
	{
		Utils::log(XorStr("CAimBot.IsTargetVisible.TraceRay Error."));
		return true;
	}

	return (trace.m_pEnt == entity || trace.fraction > 0.97f);
}

bool CAimBot::IsValidTarget(CBasePlayer * entity, bool visCheck)
{
	if (entity == nullptr || !entity->IsValid())
		return false;
	
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local && entity->GetClassID() == ET_TankRock && local->GetTeam() == 2)
		return true;

	int classId = entity->GetClassID();

	try
	{
		if (!entity->IsAlive() || classId == ET_TankRock)
			return false;
	}
	catch (...)
	{
		return false;
	}

	int team = entity->GetTeam();
	if (local != nullptr)
	{
		if (team == 4)
			return false;

		if (m_bNonFriendly && team == local->GetTeam())
			return false;

		if (team == 3 && entity->IsGhost())
			return false;
	}

	if (visCheck && m_bVisible)
	{
		if (!IsTargetVisible(entity))
			return false;
	}

	if (m_bIgnoreTank)
	{
		if (classId == ET_TANK)
			return false;
	}

	if (m_bNonWitch)
	{
		if (classId == ET_WITCH)
		{
			if (entity->GetNetProp<float>(XorStr("DT_Witch"), XorStr("m_rage")) < 1.0f)
				return false;
		}
	}

	if (m_bIgnoreCI)
	{
		if (classId == ET_INFECTED)
			return false;
	}

	if (m_bIgnoreNearSurvivor)
	{
		if (classId == ET_BOOMER && IsNearSurvivor(entity))
			return false;
	}

	return true;
}

bool CAimBot::HasValidWeapon(CBaseWeapon * weapon)
{
	if (weapon == nullptr || weapon->GetClip() <= 0)
		return false;

	int weaponId = weapon->GetWeaponID();
	if (IsNotGunWeapon(weaponId) || weaponId == Weapon_GrenadeLauncher)
		return false;

	return (weapon->GetPrimaryAttackDelay() <= 0.0f);
}

bool CAimBot::HasShotgun(CBaseWeapon* weapon)
{
	if (weapon == nullptr)
		return false;
	
	int weaponId = weapon->GetWeaponID();
	return IsShotgun(weaponId);
}

Vector CAimBot::GetTargetAimPosition(CBasePlayer* entity, std::optional<bool> visible)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || entity == nullptr || !entity->IsAlive())
		return NULL_VECTOR;
	
	bool vis = visible.value_or(m_bVisible);
	bool chestFirst = (m_bShotgunChest && HasShotgun(local->GetActiveWeapon()));
	Vector aimPosition = (chestFirst ? entity->GetChestOrigin() : entity->GetHeadOrigin());
	if (!vis || IsTargetVisible(entity, aimPosition))
		return aimPosition;

	aimPosition = (chestFirst ? entity->GetHeadOrigin() : entity->GetChestOrigin());
	if (!vis || IsTargetVisible(entity, aimPosition))
		return aimPosition;

	return NULL_VECTOR;
}

bool CAimBot::CanRunAimbot(CBasePlayer * entity)
{
	if (entity == nullptr || !entity->IsAlive() || entity->IsGhost() ||
		entity->IsHangingFromLedge() || entity->GetCurrentAttacker() != nullptr ||
		entity->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMountedGun")) ||
		entity->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMountedWeapon")) ||
		entity->IsStaggering() || entity->IsGettingUp())
		return false;

	if (entity->GetTeam() == 3)
	{
		int zombieClass = entity->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_zombieClass"));
		if (zombieClass == ZC_JOCKEY || zombieClass == ZC_SMOKER || zombieClass == ZC_CHARGER)
			return true;

		return false;
	}

	CBaseWeapon* weapon = entity->GetActiveWeapon();
	if (weapon == nullptr || !weapon->IsValid() || !weapon->CanFire()/* ||
		weapon->GetNetProp<float>(XorStr("DT_BaseAnimating"), XorStr("m_flCycle")) > 0.0f*/)
		return false;

	if ((m_bAimOfMelee && weapon->GetWeaponID() == Weapon_Melee) || weapon->IsFireGun())
		return true;

	return true;
}

Vector CAimBot::GetAimbotAimPosition(CBasePlayer* local, const Vector& eyePosition, CBasePlayer** hitEntity)
{
	Ray_t ray;
	CTraceFilter filter;
	ray.Init(eyePosition, eyePosition + m_vecAimAngles.Forward().Scale(1500.0f));
	filter.pSkip1 = local;

	trace_t trace;

	try
	{
		g_pInterface->Trace->TraceRay(ray, MASK_SHOT, &filter, &trace);
	}
	catch (...)
	{
		Utils::log(XorStr("CKnifeBot.HasEnemyVisible.TraceRay Error."));
		return INVALID_VECTOR;
	}

	if (hitEntity != nullptr)
		*hitEntity = reinterpret_cast<CBasePlayer*>(trace.m_pEnt);

	return trace.end;
}

bool CAimBot::IsFatalTarget(CBasePlayer* entity)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || entity == nullptr || !entity->IsAlive())
		return false;
	
	ZombieClass_t classId = entity->GetZombieType();
	float distance = local->GetAbsOrigin().DistTo(entity->GetAbsOrigin());
	int sequence = entity->GetSequence();

	switch (classId)
	{
		case ZC_SMOKER:
		{
			static ConVar* cvTongueRange = g_pInterface->Cvar->FindVar(XorStr("tongue_range"));
			if (distance > cvTongueRange->GetFloat())
				return false;
			
			if (sequence != ANIM_SMOKER_PULLING && sequence != ANIM_SMOKER_SHOOTING)
				return false;

			float fov = math::GetAnglesFieldOfView(entity->GetEyeAngles(), math::CalculateAim(entity->GetEyePosition(), local->GetChestOrigin()));
			if (fov > 10.0f)
				return false;

			return true;
		}
		case ZC_HUNTER:
		{
			if (distance > 100.0f)
				return false;
			
			if (!entity->GetNetProp<BYTE>(XorStr("DT_TerrorPlayer"), XorStr("m_isAttemptingToPounce")) && sequence != ANIM_HUNTER_LUNGING)
				return false;

			if (entity->GetFlags() & FL_ONGROUND)
				return false;

			float fov = math::GetAnglesFieldOfView(entity->GetVelocity().toAngles(), math::CalculateAim(entity->GetEyePosition(), local->GetChestOrigin()));
			if (fov > 15.0f)
				return false;

			return true;
		}
		case ZC_JOCKEY:
		{
			if (distance > 125.0f)
				return false;
			
			/*
			if (sequence == ANIM_JOCKEY_RIDEING)
				return false;
			*/

			if (entity->GetFlags() & FL_ONGROUND)
				return false;

			float fov = math::GetAnglesFieldOfView(entity->GetVelocity().toAngles(), math::CalculateAim(entity->GetEyePosition(), local->GetChestOrigin()));
			if (fov > 20.0f)
				return false;

			return true;
		}
		case ZC_ROCK:
		{
			if (distance > 1000.0f)
				return false;
			
			float fov = math::GetAnglesFieldOfView(entity->GetVelocity().toAngles(), math::CalculateAim(entity->GetAbsOrigin(), local->GetChestOrigin()));
			if (fov > 15.0f)
				return false;

			return true;
		}
		case ZC_CHARGER:
		{
			static ConVar* cvChargeDuration = g_pInterface->Cvar->FindVar(XorStr("z_charge_duration"));
			static ConVar* cvChargeSpeed = g_pInterface->Cvar->FindVar(XorStr("z_charge_max_speed"));
			if (distance > cvChargeDuration->GetFloat() * cvChargeSpeed->GetFloat())
				return false;
			
			if (sequence != ANIM_CHARGER_CHARGING)
				return false;

			float fov = math::GetAnglesFieldOfView(entity->GetVelocity().toAngles(), math::CalculateAim(entity->GetEyePosition(), local->GetChestOrigin()));
			if (fov > 30.0f)
				return false;

			return true;
		}
		case ZC_WITCH:
		{
			if (distance > 75.0f)
				return false;
			
			if (entity->GetNetProp<float>(XorStr("DT_Witch"), XorStr("m_rage")) < 1.0f)
				return false;

			/*
			Vector eyePosition = entity->GetAbsOrigin();
			eyePosition.z += entity->GetNetPropCollision<Vector>(XorStr("m_vecMaxs")).z;
			float fov = math::GetAnglesFieldOfView(entity->GetAbsAngles(), math::CalculateAim(eyePosition, local->GetChestOrigin()));
			if (fov > 10.0f)
				return false;
			*/

			return true;
		}
	}
	
	return false;
}

bool CAimBot::IsNearSurvivor(CBasePlayer* boomer)
{
	static ConVar* z_exploding_splat_radius = g_pInterface->Cvar->FindVar(XorStr("z_exploding_splat_radius"));

	Vector position;
	Vector origin = boomer->GetAbsOrigin();
	int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();
	float radius = z_exploding_splat_radius->GetFloat();

	for (int i = 1; i <= maxEntity; ++i)
	{
		CBaseEntity* entity = reinterpret_cast<CBaseEntity*>(g_pInterface->EntList->GetClientEntity(i));
		if (entity == nullptr || !entity->IsValid())
			continue;

		position = entity->GetAbsOrigin();

		// 爆炸比较特殊，不好进行墙壁检测，只能假设可以触发了
		if (origin.DistTo(position) > radius)
			continue;

		if (entity->IsPlayer() && entity->GetTeam() == 2)
		{
			CBasePlayer* player = reinterpret_cast<CBasePlayer*>(entity);
			if (player->IsAlive() && !player->GetCurrentAttacker())
				return true;
		}

		int classId = entity->GetClassID();
		if (classId == ET_WITCH)
		{
			if (entity->GetNetProp<float>(XorStr("DT_Witch"), XorStr("m_rage")) < 1.0f)
				return true;
		}

		// TODO: 检查警报车。服务器和客户端的 classname 并不对应
	}

	return false;
}
