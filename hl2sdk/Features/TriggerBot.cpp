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

	// QAngle viewAngles;
	// g_pInterface->Engine->GetViewAngles(viewAngles);
	GetAimTarget(cmd->viewangles);

	if (!player->IsAlive() || player->GetCurrentAttacker() != nullptr || player->IsHangingFromLedge())
		return;

	CBaseWeapon* weapon = player->GetActiveWeapon();
	if (m_pAimTarget == nullptr || weapon == nullptr || !weapon->IsFireGun() || !weapon->CanFire())
		return;

	// 检查目标是否可以被攻击
	int team = m_pAimTarget->GetTeam();
	int classId = m_pAimTarget->GetClassID();
	if (team == 4 || (team == 3 && classId != ET_TankRock && m_pAimTarget->IsGhost()))
		return;

	// 检查队友是否被控
	if (team == 2)
	{
		CBasePlayer* attacker = m_pAimTarget->GetCurrentAttacker();
		if (attacker == nullptr || !attacker->IsAlive())
			return;

		// 生还者被控时被队友射击受伤的会是特感
		// 所以这里应该瞄准的是特感
		if(math::GetVectorDistance(m_pAimTarget->GetAbsOrigin(), attacker->GetAbsOrigin(), true) <= 50.0f)
			m_pAimTarget = attacker;
	}

	// 打蹲下未愤怒 Witch 的头可以造成硬直效果
	// 这个用于辅助瞄准头部
	bool canWitchHeadshot = false;
	if (classId == ET_WITCH)
	{
		float angry = m_pAimTarget->GetNetProp<float>(XorStr("DT_Witch"), XorStr("m_rage"));

		// 主动攻击蹲下的 Witch
		if (angry <= 0.0f && m_pAimTarget->GetSequence() == 4 && (cmd->buttons & IN_ATTACK))
		{
			// 蹲妹爆头可以打出硬直
			canWitchHeadshot = true;
		}

		// Witch 在被惊扰后被攻击不会转移目标，除非点燃它
		if (!canWitchHeadshot && (m_bNonWitch || angry < 1.0f))
			return;

		// 检查是否会点燃 Witch, 防止意外
		// m_upgradeBitVec, 1=燃烧.2=高爆.4=激光
		// m_nUpgradedPrimaryAmmoLoaded, 子弹数量
		if (m_pAimTarget->GetNetProp<byte>(XorStr("DT_Infected"), XorStr("m_bIsBurning")) == 0 &&
			(weapon->GetNetProp<DWORD>(XorStr("DT_TerrorGun"), XorStr("m_upgradeBitVec")) & 1) &&
			weapon->GetNetProp<byte>(XorStr("DT_TerrorGun"), XorStr("m_nUpgradedPrimaryAmmoLoaded")) > 0)
			return;
	}

	if (m_bBlockFriendlyFire && team == player->GetTeam() &&
		!player->IsIncapacitated() && player->GetCurrentAttacker() == nullptr)
	{
		cmd->buttons &= ~IN_ATTACK;
		return;
	}

	// 开枪
	cmd->buttons |= IN_ATTACK;

	if (classId == ET_TankRock)
		return;

	if (m_bTraceWithoutMagnum && classId == ET_INFECTED && weapon->GetWeaponID() == WeaponId_MagnumPistol)
		return;

	Vector myEyeOrigin = player->GetEyePosition();
	if (m_bTraceVelExt)
		myEyeOrigin = math::VelocityExtrapolate(myEyeOrigin, player->GetVelocity(), m_bForwardtrack);

	if (m_bFollowEnemy)
	{
		Vector aimOrigin = m_pAimTarget->GetHitboxOrigin(m_iHitBox);
		if (!m_bFollowVisible || IsVisableToPosition(player, m_pAimTarget, aimOrigin))
		{
			QAngle aimAngles = math::CalculateAim(myEyeOrigin, aimOrigin);
			if (math::GetAnglesFieldOfView(cmd->viewangles, aimAngles) <= m_fFollowFov)
			{
				g_pInterface->Engine->SetViewAngles(aimAngles);
			}
		}
	}

	if (m_bTraceHead || canWitchHeadshot)
	{
		Vector aimHeadOrigin = (m_bTraceShotgunChest && HasShotgun(weapon) ? m_pAimTarget->GetChestOrigin() : m_pAimTarget->GetHeadOrigin());
		if (!m_bFollowVisible || IsVisableToPosition(player, m_pAimTarget, aimHeadOrigin))
		{
			if (m_bTraceVelExt)
				aimHeadOrigin = math::VelocityExtrapolate(aimHeadOrigin, m_pAimTarget->GetVelocity(), m_bTraceForwardtrack);

			QAngle aimAngles = math::CalculateAim(myEyeOrigin, aimHeadOrigin);
			if (math::GetAnglesFieldOfView(cmd->viewangles, aimAngles) <= m_fTraceFov)
			{
				if (m_bTraceSilent || canWitchHeadshot)
					g_pViewManager->ApplySilentFire(aimAngles);
				else
					g_pInterface->Engine->SetViewAngles(aimAngles);
			}
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
	IMGUI_TIPS("显示一个准星，根据瞄准的敌人切换不同的颜色。");

	ImGui::Checkbox(XorStr("Block Friendly Fire"), &m_bBlockFriendlyFire);
	IMGUI_TIPS("防止黑枪，瞄准队友时禁止开枪。");

	ImGui::Checkbox(XorStr("Trigger No Witchs"), &m_bNonWitch);
	IMGUI_TIPS("自动开枪不会射击Witch。");

	ImGui::Checkbox(XorStr("Trigger Position"), &m_bAimPosition);
	IMGUI_TIPS("显示瞄准的位置，调试用。");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Trigger Velocity Extrapolate"), &m_bVelExt);
	IMGUI_TIPS("自动开枪速度预测。");

	ImGui::Checkbox(XorStr("Trigger Forwardtrack"), &m_bForwardtrack);
	IMGUI_TIPS("自动开枪速度延迟预测。");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Track head"), &m_bTraceHead);
	IMGUI_TIPS("自动开枪时射击头部，用于 猎头者 模式。");

	ImGui::Checkbox(XorStr("Track Visable"), &m_bTraceVisible);
	IMGUI_TIPS("自动开枪时射击头部需要检查是否可见。");

	ImGui::Checkbox(XorStr("Track Silent"), &m_bTraceSilent);
	IMGUI_TIPS("自动开枪时射击头部防止被观察者发现。\n建议开启，因为不开是打不准的。");

	ImGui::Checkbox(XorStr("Track Velocity Extrapolate"), &m_bTraceVelExt);
	IMGUI_TIPS("自动开枪时射击头部速度预测。");

	ImGui::Checkbox(XorStr("Track Forwardtrack"), &m_bTraceForwardtrack);
	IMGUI_TIPS("自动开枪时射击头部速度延迟预测。");

	ImGui::SliderFloat(XorStr("Track FOV"), &m_fTraceFov, 1.0f, 90.0f, ("%.1f"));
	IMGUI_TIPS("瞄准头部范围限制。");

	ImGui::Checkbox(XorStr("Track Without Magnum"), &m_bTraceWithoutMagnum);
	IMGUI_TIPS("自动开枪时射击普感头部忽略马格南。");
	
	ImGui::Checkbox(XorStr("Track Shotgun Chest"), &m_bTraceShotgunChest);
	IMGUI_TIPS("持有霰弹枪时瞄准位置改为身体。");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Follow the target"), &m_bFollowEnemy);
	IMGUI_TIPS("自动开枪尝试跟随敌人。");

	ImGui::Checkbox(XorStr("Follow Visible"), &m_bFollowVisible);
	IMGUI_TIPS("自动开枪尝试跟随敌人需要检查是否可见。");

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
	m_bTraceVelExt = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_track_velext"), m_bTraceVelExt);
	m_bVelExt = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_velext"), m_bVelExt);
	m_bForwardtrack = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_forwardtrack"), m_bForwardtrack);
	m_bTraceForwardtrack = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_track_forwardtrack"), m_bTraceForwardtrack);
	m_bAimPosition = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_aimpos"), m_bAimPosition);
	m_bTraceVisible = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_track_visible"), m_bTraceVisible);
	m_bFollowVisible = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_follow_visible"), m_bFollowVisible);
	m_bTraceWithoutMagnum = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_track_magnum"), m_bTraceWithoutMagnum);
	m_bTraceShotgunChest = g_pConfig->GetBoolean(mainKeys, XorStr("trigger_shotgun_chest"), m_bTraceShotgunChest);
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
	g_pConfig->SetValue(mainKeys, XorStr("trigger_track_velext"), m_bTraceVelExt);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_velext"), m_bVelExt);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_forwardtrack"), m_bForwardtrack);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_track_forwardtrack"), m_bTraceForwardtrack);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_aimpos"), m_bAimPosition);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_track_visible"), m_bTraceVisible);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_follow_visible"), m_bFollowVisible);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_track_magnum"), m_bTraceWithoutMagnum);
	g_pConfig->SetValue(mainKeys, XorStr("trigger_shotgun_chest"), m_bTraceShotgunChest);
}

void CTriggerBot::OnEnginePaint(PaintMode_t mode)
{
	if (!m_bActive)
		return;
	
	if (!m_bCrosshairs && !m_bAimPosition)
		return;
	
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive())
		return;

	D3DCOLOR color = CDrawing::WHITE;
	if (m_pAimTarget == nullptr || !m_pAimTarget->IsAlive())
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

			if(m_pAimTarget && m_pAimTarget->IsAlive())
				g_pDrawing->DrawText(screen.x, screen.y - 16, CDrawing::CYAN, true, XorStr("i=%d, h=%d"), m_pAimTarget->GetIndex(), m_pAimTarget->GetHealth());
		}
	}
}

class CTriggerTraceFilter : public CTraceFilter
{
public:
	virtual bool ShouldHitEntity(CBaseEntity* pEntityHandle, int contentsMask) override
	{
		if (pEntityHandle == pSkip1)
			return false;
		
		try
		{
			return (pEntityHandle->GetClassID() != ET_SurvivorRescue);
		}
		catch (...)
		{
			return true;
		}

		return true;
	}
};

CBasePlayer * CTriggerBot::GetAimTarget(const QAngle& eyeAngles)
{
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	
	CTriggerTraceFilter filter;
	filter.pSkip1 = player;

	Ray_t ray;
	Vector startPosition = player->GetEyePosition();
	if (m_bVelExt)
		startPosition = math::VelocityExtrapolate(startPosition, player->GetVelocity(), m_bForwardtrack);

	Vector endPosition = startPosition + eyeAngles.Forward().Scale(3500.0f);
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

bool CTriggerBot::HasShotgun(CBaseWeapon* weapon)
{
	if (weapon == nullptr)
		return false;

	int weaponId = weapon->GetWeaponID();
	return IsShotgun(weaponId);
}

bool CTriggerBot::IsVisableToPosition(CBasePlayer * local, CBasePlayer * target, const Vector & position)
{
	Ray_t ray;
	ray.Init(local->GetEyePosition(), position);

	CTriggerTraceFilter filter;
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

	return (trace.m_pEnt == target || trace.fraction > 0.97f);
}
