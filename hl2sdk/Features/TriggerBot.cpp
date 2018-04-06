#include "TriggerBot.h"
#include "NoRecoilSpread.h"
#include "../Utils/math.h"
#include "../interfaces.h"
#include "../hook.h"
#include "../../l4d2Simple2/config.h"

CTriggerBot* g_pTriggerBot = nullptr;

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
	if (player == nullptr)
		return;

	QAngle viewAngles;
	g_pInterface->Engine->GetViewAngles(viewAngles);
	GetAimTarget(viewAngles);

	if (!player->IsAlive() || player->GetAttacker() != nullptr || player->IsHangingFromLedge())
		return;

	CBaseWeapon* weapon = player->GetActiveWeapon();
	if (m_pAimTarget == nullptr || weapon == nullptr || !weapon->IsFireGun() || !weapon->CanFire())
		return;

	// 非玩家生还者是免疫僵尸攻击的
	// 而且他们还会和玩家抢东西(抢包抢药抢武器)
	// 例如 c6m3, c6m1 的一代生还者
	if (m_pAimTarget->GetTeam() == 2)
	{
		CBasePlayer* attacker = m_pAimTarget->GetAttacker();
		if (attacker == nullptr || !attacker->IsAlive())
			return;

		// 生还者被控时被队友射击受伤的会是特感
		// 所以这里应该瞄准的是特感
		if(math::GetVectorDistance(m_pAimTarget->GetAbsOrigin(), attacker->GetAbsOrigin(), true) <= 50.0f)
			m_pAimTarget = attacker;
	}

	if (m_pAimTarget->GetClassID() == ET_WITCH)
	{
		// Witch 在被惊扰后被攻击不会转移目标，除非点燃它
		if (m_bNonWitch || m_pAimTarget->GetNetProp<float>(XorStr("DT_Witch"), XorStr("m_rage")) < 1.0f)
			return;

		// 检查是否会点燃 Witch, 防止意外转移目标
		// m_upgradeBitVec, 1=燃烧.2=高爆.4=激光
		// m_nUpgradedPrimaryAmmoLoaded, 子弹数量
		if (m_pAimTarget->GetNetProp<byte>(XorStr("DT_Infected"), XorStr("m_bIsBurning")) == 0 &&
			(weapon->GetNetProp<DWORD>(XorStr("DT_TerrorGun"), XorStr("m_upgradeBitVec")) & 1) &&
			weapon->GetNetProp<byte>(XorStr("DT_TerrorGun"), XorStr("m_nUpgradedPrimaryAmmoLoaded")) > 0)
			return;
	}

	if (m_bBlockFriendlyFire && m_pAimTarget->GetTeam() == player->GetTeam() &&
		!player->IsIncapacitated() && player->GetAttacker() == nullptr)
	{
		cmd->buttons &= ~IN_ATTACK;
		return;
	}

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
			if(m_bTraceSilent)
				g_pViewManager->ApplySilentFire(aimAngles);
			else
				g_pInterface->Engine->SetViewAngles(aimAngles);
		}
	}
}

void CTriggerBot::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("TriggerBot")))
		return;

	ImGui::Checkbox(XorStr("Trigger Allow"), &m_bActive);
	IMGUI_TIPS("自动开枪。");

	ImGui::Checkbox(XorStr("Trigger Crosshairs"), &m_bCrosshairs);
	ImGui::Checkbox(XorStr("Block Friendly Fire"), &m_bBlockFriendlyFire);
	IMGUI_TIPS("防止黑枪，瞄准队友时禁止开枪。");

	ImGui::Checkbox(XorStr("Trigger No Witchs"), &m_bNonWitch);
	IMGUI_TIPS("自动开枪不会射击队友。");

	ImGui::Checkbox(XorStr("Trigger Position"), &m_bAimPosition);

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Track head"), &m_bTraceHead);
	IMGUI_TIPS("自动开枪时射击头部，用于 猎头者 模式。");

	ImGui::Checkbox(XorStr("Track Silent"), &m_bTraceSilent);
	IMGUI_TIPS("自动开枪时射击头部防止被观察者发现。");

	ImGui::SliderFloat(XorStr("Track FOV"), &m_fTraceFov, 1.0f, 90.0f, ("%.1f"));

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Follow the target"), &m_bFollowEnemy);
	IMGUI_TIPS("自动开枪尝试跟随敌人。");

	ImGui::SliderFloat(XorStr("Follow FOV"), &m_fFollowFov, 1.0f, 90.0f, ("%.1f"));
	IMGUI_TIPS("自动开枪尝试跟随敌人范围。");

	ImGui::TreePop();
}

void CTriggerBot::OnConfigLoading(const config_type & data)
{
	const std::string mainKeys = XorStr("Triggerbots");
	
	m_bActive = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_enable"), m_bActive);
	m_bCrosshairs = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_crosshair"), m_bCrosshairs);
	m_bBlockFriendlyFire = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_non_friendly"), m_bBlockFriendlyFire);
	m_bNonWitch = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_non_witch"), m_bNonWitch);
	m_bTraceHead = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_track_head"), m_bTraceHead);
	m_bTraceSilent = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_track_silent"), m_bTraceSilent);
	m_fTraceFov = g_pConfig->GetFloat(mainKeys, XorStr("trigger_track_fov"), m_fTraceFov);
	m_bFollowEnemy = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_follow"), m_bFollowEnemy);
	m_fFollowFov = g_pConfig->GetFloat(mainKeys, XorStr("trigger_follow_fov"), m_fFollowFov);
}

void CTriggerBot::OnConfigSave(config_type & data)
{
	const std::string mainKeys = XorStr("Triggerbots");

	g_pConfig->SetValue(mainKeys, XorStr("trigger_enable"), m_bActive);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_crosshair"), m_bCrosshairs);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_non_friendly"), m_bBlockFriendlyFire);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_non_witch"), m_bNonWitch);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_track_head"), m_bTraceHead);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_track_silent"), m_bTraceSilent);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_track_fov"), m_fTraceFov);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_follow"), m_bFollowEnemy);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_follow_fov"), m_fFollowFov);
}

void CTriggerBot::OnEnginePaint(PaintMode_t mode)
{
	if (!m_bCrosshairs && !m_bAimPosition)
		return;
	
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr)
		return;

	D3DCOLOR color = CDrawing::WHITE;
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

	if (m_bCrosshairs)
	{
		int width, height;
		g_pInterface->Engine->GetScreenSize(width, height);
		width /= 2;
		height /= 2;

		// g_pInterface->Surface->DrawLine(width - 5, height, width + 5, height);
		// g_pInterface->Surface->DrawLine(width, height - 5, width, height + 5);
		g_pDrawing->DrawLine(width - 10, height, width + 10, height, color);
		g_pDrawing->DrawLine(width, height - 10, width, height + 10, color);
	}

	if (m_bAimPosition)
	{
		Vector screen;
		if (math::WorldToScreenEx(m_vecAimOrigin, screen))
		{
			g_pDrawing->DrawText(static_cast<int>(screen.x), static_cast<int>(screen.y),
				color, true, XorStr("O"));
		}
	}
}

CBasePlayer * CTriggerBot::GetAimTarget(const QAngle& eyeAngles)
{
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	
	CTraceFilter filter;
	filter.pSkip1 = player;

	Ray_t ray;
	Vector startPosition = player->GetEyePosition();
	Vector endPosition = startPosition + eyeAngles.Forward().Scale(3500.0f);
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

	m_vecAimOrigin = trace.end;

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

	int weaponId = weapon->GetWeaponID();
	if (IsNotGunWeapon(weaponId) || weaponId == WeaponId_GrenadeLauncher)
		return false;

	return true;
}
