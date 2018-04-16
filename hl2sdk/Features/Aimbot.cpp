#include "Aimbot.h"
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

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (!CanRunAimbot(local) || (m_bOnFire && !(cmd->buttons & IN_ATTACK)))
	{
		m_bRunAutoAim = false;
		return;
	}

	// QAngle viewAngles;
	// g_pInterface->Engine->GetViewAngles(viewAngles);
	if(!IsValidTarget(m_pAimTarget) || !(cmd->buttons & IN_ATTACK))
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

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Distance priority"), &m_bDistance);
	IMGUI_TIPS("优先选择最接距离近的目标，如果不开启则优先选择最接近准星的目标。");

	ImGui::SliderFloat(XorStr("Aimbot Fov"), &m_fAimFov, 1.0f, 360.0f, XorStr("%.0f"));
	IMGUI_TIPS("自动瞄准范围（半径，从准星位置开始）");

	ImGui::SliderFloat(XorStr("Aimbot Distance"), &m_fAimDist, 1.0f, 5000.0f, XorStr("%.0f"));
	IMGUI_TIPS("自动瞄准范围（距离）");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("AutoAim Range"), &m_bShowRange);
	IMGUI_TIPS("待修复。");

	ImGui::Checkbox(XorStr("AutoAim Angles"), &m_bShowAngles);
	IMGUI_TIPS("显示瞄准的位置。");

	ImGui::TreePop();
}

void CAimBot::OnConfigLoading(const config_type & data)
{
	const std::string mainKeys = XorStr("Aimbot");
	
	m_bActive = g_pConfig->GetBoolean(mainKeys, XorStr("autoaim_enable"), m_bActive);
	m_bOnFire = g_pConfig->GetBoolean(mainKeys, XorStr("autoaim_only_shot"), m_bOnFire);
	m_bSilent = g_pConfig->GetBoolean(mainKeys, XorStr("autoaim_silent"), m_bSilent);
	m_bPerfectSilent = g_pConfig->GetBoolean(mainKeys, XorStr("autoaim_perfect_silent"), m_bPerfectSilent);
	m_bVisible = g_pConfig->GetBoolean(mainKeys, XorStr("autoaim_visible"), m_bVisible);
	m_bNonFriendly = g_pConfig->GetBoolean(mainKeys, XorStr("autoaim_non_friendly"), m_bNonFriendly);
	m_bNonWitch = g_pConfig->GetBoolean(mainKeys, XorStr("autoaim_non_witch"), m_bNonWitch);
	m_bDistance = g_pConfig->GetBoolean(mainKeys, XorStr("autoaim_distance_priority"), m_bDistance);
	m_fAimFov = g_pConfig->GetFloat(mainKeys, XorStr("autoaim_fov"), m_fAimFov);
	m_fAimDist = g_pConfig->GetFloat(mainKeys, XorStr("autoaim_distance"), m_fAimDist);
	m_bShowRange = g_pConfig->GetFloat(mainKeys, XorStr("autoaim_show_range"), m_bShowRange);
	m_bShowAngles = g_pConfig->GetFloat(mainKeys, XorStr("autoaim_show_angles"), m_bShowAngles);
}

void CAimBot::OnConfigSave(config_type & data)
{
	const std::string mainKeys = XorStr("Aimbot");
	
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_enable"), m_bActive);
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_only_shot"), m_bOnFire);
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_silent"), m_bSilent);
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_perfect_silent"), m_bPerfectSilent);
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_visible"), m_bVisible);
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_non_friendly"), m_bNonFriendly);
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_non_witch"), m_bNonWitch);
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_distance_priority"), m_bDistance);
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_fov"), m_fAimFov);
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_distance"), m_fAimDist);
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_show_range"), m_bShowRange);
	g_pConfig->SetValue(mainKeys, XorStr("autoaim_show_angles"), m_bShowAngles);
}

void CAimBot::OnEnginePaint(PaintMode_t mode)
{
	if (!m_bShowRange)
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr)
		return;

	int width = 0, height = 0;
	g_pInterface->Engine->GetScreenSize(width, height);

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

	if(m_bRunAutoAim)
		g_pDrawing->DrawCircle(width, height, static_cast<int>(radius), CDrawing::GREEN);
	else
		g_pDrawing->DrawCircle(width, height, static_cast<int>(radius), CDrawing::WHITE);

	g_pDrawing->DrawText(width, height - 32, CDrawing::YELLOW, true, XorStr("aimFov = %.0f"), m_fTargetFov);
	g_pDrawing->DrawText(width, height - 16, CDrawing::ORANGE, true, XorStr("aimDistance = %.0f"), m_fTargetDistance);
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

	g_pInterface->DebugOverlay->AddLineOverlay(eyePosition, aimPosition, 64, 128, 128, true, 0.1f);

	Vector screenPosition;
	if (hitEntity != nullptr && math::WorldToScreenEx(aimPosition, screenPosition))
		g_pDrawing->DrawText(static_cast<int>(screenPosition.x), static_cast<int>(screenPosition.y),
			CDrawing::PURPLE, true, "X");
}

CBasePlayer * CAimBot::FindTarget(const QAngle& myEyeAngles)
{
	m_pAimTarget = nullptr;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr)
		return nullptr;
	
	Vector myEyePosition = local->GetEyePosition();

	float minFov = m_fAimFov + 1.0f, minDistance = m_fAimDist + 1.0f;
	int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();

	// 特感和普感是同一个阵营的
	if (local->GetTeam() == 3)
		maxEntity = 64;

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
				minFov = fov;
			}
		}
		else
		{
			// 范围优先
			if (dist <= m_fAimDist && fov < minFov)
			{
				m_pAimTarget = entity;
				minDistance = dist;
				minFov = fov;
			}
		}

		// 如果已经选择了玩家敌人，则不需要再去选择普感敌人
		if (i > 64 && m_pAimTarget != nullptr)
			break;
	}

	m_fTargetFov = minFov;
	m_fTargetDistance = minDistance;
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
	if (entity == nullptr)
		return false;
	
	try
	{
		if (!entity->IsAlive())
			return false;
	}
	catch (...)
	{
		return false;
	}

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

	int weaponId = weapon->GetWeaponID();
	if (IsNotGunWeapon(weaponId) || weaponId == Weapon_GrenadeLauncher)
		return false;

	return (weapon->GetPrimaryAttackDelay() <= 0.0f);
}

bool CAimBot::CanRunAimbot(CBasePlayer * entity)
{
	if (entity == nullptr || !entity->IsAlive())
		return false;

	if (entity->GetTeam() == 3)
	{
		int zombieClass = entity->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_zombieClass"));
		if (zombieClass == ZC_JOCKEY || zombieClass == ZC_SMOKER || zombieClass == ZC_CHARGER)
			return true;

		return false;
	}

	CBaseWeapon* weapon = entity->GetActiveWeapon();
	if (weapon == nullptr || !weapon->IsFireGun() || !weapon->CanFire())
		return false;

	return true;
}

Vector CAimBot::GetAimPosition(CBasePlayer* local, const Vector& eyePosition, CBasePlayer** hitEntity)
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
		return false;
	}

	if (hitEntity != nullptr)
		*hitEntity = reinterpret_cast<CBasePlayer*>(trace.m_pEnt);

	return trace.end;
}
