#include "QTE.h"
#include "../hook.h"
#include "../Utils/math.h"
#include "NoRecoilSpread.h"
#include "../../l4d2Simple2/config.h"

CQuickTriggerEvent* g_pQTE = nullptr;
#define IsShotgun(_id)				(_id == WeaponId_PumpShotgun || _id == WeaponId_Chrome || _id == WeaponId_AutoShotgun || _id == WeaponId_SPAS)
#define HITBOX_WITCH_CHEST			8
#define ANIM_CHARGER_CHARGING		5
#define ANIM_SMOKER_PULLING			30
#define ANIM_SMOKER_SHOOTING		27
#define ANIM_HUNTER_LUNGING			67
#define ANIM_JOCKEY_LEAPING			10
#define ANIM_JOCKEY_RIDEING			8

CQuickTriggerEvent::CQuickTriggerEvent() : CBaseFeatures::CBaseFeatures()
{
	/*
	for (int i = 0; i < 65; ++i)
		m_StartAttackTime[i] = std::chrono::system_clock::from_time_t(0);

	m_WaitTime = std::chrono::system_clock::from_time_t(0);
	*/
}

CQuickTriggerEvent::~CQuickTriggerEvent()
{
	CBaseFeatures::~CBaseFeatures();
}

void CQuickTriggerEvent::OnCreateMove(CUserCmd * cmd, bool*)
{
	if (!m_bActive || m_bMenuOpen)
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || local->GetTeam() != 2 || !local->IsAlive() ||
		local->IsIncapacitated() || local->IsHangingFromLedge() ||
		local->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMountedGun")) ||
		local->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMountedWeapon")))
		return;

	/*
	CBasePlayer* player = m_pSmokerAttacker;
	m_pSmokerAttacker = nullptr;
	*/

	CBasePlayer* player = m_pLastTarget;
	CBasePlayer* dominater = local->GetCurrentAttacker();

	// 被舌头拉但还有反抗时间时也是能获取到攻击者的
	if (dominater != nullptr && dominater->IsAlive() && dominater->GetZombieType() != ZC_SMOKER/* && dominater != player*/)
		return;

	CBaseWeapon* weapon = local->GetActiveWeapon();
	if (weapon == nullptr)
		return;

	float distance = 1000.0f;
	int weaponId = weapon->GetWeaponID();
	bool canFire = (weapon->IsFireGun() && weapon->CanFire());
	bool hasMelee = (weaponId == Weapon_Melee);
	bool canShove = local->CanShove();
	Vector myOrigin = local->GetEyePosition();

	// 这里好像没有效果。。。
	/*
	CBaseHandle handle = local->GetNetProp<CBaseHandle>(XorStr("DT_TerrorPlayer"), XorStr("m_tongueOwner"));
	if (handle.IsValid() && !local->GetNetProp<BYTE>(XorStr("DT_TerrorPlayer"), XorStr("m_isProneTongueDrag")))
	{
		player = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntityFromHandle(handle));
		if (player != nullptr)
		{
			distance = myOrigin.DistTo(player->GetAbsOrigin());
			
			if (hasMelee)
				HandleMeleeSelfClear(local, player, cmd, distance, weapon);
			else if (canFire)
				HandleShotSelfClear(local, player, cmd, distance);
			else
				HandleShoveSelfClear(local, player, cmd, distance);

			// 优先处理被拉自救，其他的先不管了
			return;
		}
	}
	*/

	QAngle viewAngles;
	g_pInterface->Engine->GetViewAngles(viewAngles);

	if (player == nullptr)
		player = FindTarget(local, viewAngles);
	if (player == nullptr)
		return;

	// 计算距离时进行近战预测，实际攻击时不预测
	Vector targetOrigin = GetTargetAimPosition(player, false);
	if (!targetOrigin.IsValid())
		return;

	// 近战的攻击判断很迷，很难击中目标。这里改成推，成功率更高
	float fov = math::GetAnglesFieldOfView(viewAngles, math::CalculateAim(myOrigin, targetOrigin));
	if (hasMelee && m_bMeleeUnslienced && fov > m_fMeleeUnsliencedFov)
		hasMelee = false;

	int tickToPred = 1;
	if (m_bSwingExt)
	{
		if (hasMelee && weapon->CanFire())
			tickToPred = TIME_TO_TICKS(weapon->GetMeleeDelay());
		if (tickToPred < 1)
			tickToPred = 1;
	}

	if (m_bVelExt)
		myOrigin = math::VelocityExtrapolate(myOrigin, local->GetVelocity(), m_bLagExt, tickToPred);

	if (m_bVelExt)
		targetOrigin = math::VelocityExtrapolate(targetOrigin, local->GetVelocity(), m_bLagExt, tickToPred);

	distance = myOrigin.DistTo(targetOrigin);
	ZombieClass_t classId = player->GetZombieType();

	switch (classId)
	{
		case ZC_SMOKER:
		{
			if (canShove && m_bCanShoveSmoker)
			{
				HandleShoveSelfClear(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("shove %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			else if (hasMelee && m_bCanMeleeSmoker)
			{
				// 近战有延迟的，所以需要提前
				HandleMeleeSelfClear(local, player, cmd, distance, weapon);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("melee attack %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			else if (canFire && m_bCanShotSmoker)
			{
				HandleShotSelfClear(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("gun shot %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			break;
		}
		case ZC_HUNTER:
		{
			if (canShove && m_bCanShoveHunter)
			{
				HandleShoveSelfClear(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("shove %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			else if (hasMelee && m_bCanMeleeHunter)
			{
				// 近战有延迟的，所以需要提前
				HandleMeleeSelfClear(local, player, cmd, distance / m_fMeleeScale, weapon);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("melee attack %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			else if (canFire && m_bCanShotHunter)
			{
				HandleShotSelfClear(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("gun shot %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			break;
		}
		case ZC_JOCKEY:
		{
			if (canShove && m_bCanShoveJockey)
			{
				// 推猴子必须提前
				HandleShoveSelfClear(local, player, cmd, (classId == ZC_JOCKEY ? distance / m_fJockeyScale : distance));
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("shove %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			else if (hasMelee && m_bCanMeleeJockey)
			{
				// 近战有延迟的，所以需要提前
				HandleMeleeSelfClear(local, player, cmd, distance / m_fJockeyScale / m_fMeleeScale, weapon);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("melee attack %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			else if (canFire && m_bCanShotJockey)
			{
				HandleShotSelfClear(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("gun shot %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			break;
		}
		case ZC_CHARGER:
		{
			if (hasMelee && m_bCanMeleeCharger)
			{
				// 近战有延迟的，所以需要提前
				HandleMeleeSelfClear(local, player, cmd, distance / m_fMeleeScale, weapon);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("melee attack %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			else if (canFire && m_bCanShotCharger)
			{
				HandleShotSelfClear(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("gun shot %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			break;
		}
		case ZC_ROCK:
		{
			if (hasMelee && m_bCanMeleeRock)
			{
				// 近战有延迟的，所以需要提前
				HandleMeleeSelfClear(local, player, cmd, distance / m_fMeleeScale, weapon);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("melee attack charger/rock(%d), dist: %.0f"), classId, distance);
			}
			else if (canFire && m_bCanShotRock)
			{
				HandleShotSelfClear(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("gun shot charger/rock(%d), dist: %.0f"), classId, distance);
			}
			break;
		}
		case ZC_WITCH:
		{
			if (canFire && IsShotgun(weaponId))
			{
				HandleWitchCrown(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("gun shot witch, dist: %.0f"), distance);
			}
			break;
		}
		case ZC_BOOMER:
		case ZC_SPITTER:
		{
			if (canShove && !player->IsStaggering())
			{
				HandleShoveSelfClear(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("shove boomer/spitter(%d), dist: %.0f"), classId, distance);
			}
			break;
		}
	}
}

void CQuickTriggerEvent::OnEntityDeleted(CBaseEntity* entity)
{
	if (m_pLastTarget == entity)
		m_pLastTarget = nullptr;
}

void CQuickTriggerEvent::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("QuickTriggerEvent")))
		return;

	ImGui::Checkbox(XorStr("AutoQTE"), &m_bActive);
	IMGUI_TIPS("紧急自救");

	ImGui::Checkbox(XorStr("Velocity Extrapolate"), &m_bVelExt);
	IMGUI_TIPS("速度预测");

	ImGui::Checkbox(XorStr("Forwardtrack"), &m_bLagExt);
	IMGUI_TIPS("延迟预测");
	
	ImGui::Checkbox(XorStr("SwingPrediction"), &m_bSwingExt);
	IMGUI_TIPS("近战前摇预测");

	ImGui::Checkbox(XorStr("Silent Aim"), &m_bSilent);
	IMGUI_TIPS("自己看不到自动瞄准，建议开启");

	ImGui::Checkbox(XorStr("Perfect Silent"), &m_bPerfectSilent);
	IMGUI_TIPS("观察者看不到自动瞄准，建议开启");

	ImGui::Checkbox(XorStr("Visible Only"), &m_bOnlyVisible);
	IMGUI_TIPS("可见检查");
	
	ImGui::Checkbox(XorStr("Target Check"), &m_bCheckFov);
	IMGUI_TIPS("目标检查");
	
	ImGui::Checkbox(XorStr("Melee TickMode"), &m_bMeleeAsShove);
	IMGUI_TIPS("在 Silent 模式下必须开启");
	
	ImGui::Checkbox(XorStr("Show Info"), &m_bLogInfo);
	IMGUI_TIPS("显示信息");

	ImGui::SliderFloat(XorStr("Shove Range Extra"), &m_fShoveDstExtra, 1.0f, 300.0f, ("%.1f"));
	IMGUI_TIPS("推预测范围");

	ImGui::SliderFloat(XorStr("Melee Range Extra"), &m_fMeleeDstExtra, 1.0f, 300.0f, ("%.1f"));
	IMGUI_TIPS("近战预测范围");
	
	ImGui::SliderInt(XorStr("Shove Ticks"), &m_iShoveTicks, 1, 15);
	IMGUI_TIPS("推 tick 数量");

	ImGui::Checkbox(XorStr("Melee Unslienced"), &m_bMeleeUnslienced);
	IMGUI_TIPS("近战不使用 Slient 模式(解决刀不中的问题)");

	ImGui::SliderFloat(XorStr("Melee Unslienced FOV"), &m_fMeleeUnsliencedFov, 1.0f, 90.0f, ("%.1f"));
	IMGUI_TIPS("近战不使用 Slient 模式视野");

	ImGui::Separator();

	ImGui::Checkbox(XorStr("Allow Shot Smoker"), &m_bCanShotSmoker);
	IMGUI_TIPS("开启射击舌头");
	
	ImGui::Checkbox(XorStr("Allow Shot Hunter"), &m_bCanShotHunter);
	IMGUI_TIPS("开启射击猎人");
	
	ImGui::Checkbox(XorStr("Allow Shot Jockey"), &m_bCanShotJockey);
	IMGUI_TIPS("开启射击猴子");
	
	ImGui::Checkbox(XorStr("Allow Shot Charger"), &m_bCanShotCharger);
	IMGUI_TIPS("开启射击牛");
	
	ImGui::Checkbox(XorStr("Allow Shot Rock"), &m_bCanShotRock);
	IMGUI_TIPS("开启射击坦克石头");
	
	ImGui::Checkbox(XorStr("Allow Cut Smoker"), &m_bCanMeleeSmoker);
	IMGUI_TIPS("开启刀舌头");
	
	ImGui::Checkbox(XorStr("Allow Cut Hunter"), &m_bCanMeleeHunter);
	IMGUI_TIPS("开启刀猎人");
	
	ImGui::Checkbox(XorStr("Allow Cut Jockey"), &m_bCanMeleeJockey);
	IMGUI_TIPS("开启刀猴子");
	
	ImGui::Checkbox(XorStr("Allow Cut Charger"), &m_bCanMeleeCharger);
	IMGUI_TIPS("开启刀牛");
	
	ImGui::Checkbox(XorStr("Allow Cut Rock"), &m_bCanMeleeRock);
	IMGUI_TIPS("开启刀石头");
	
	ImGui::Checkbox(XorStr("Allow Shove Hunter"), &m_bCanShoveHunter);
	IMGUI_TIPS("开启推猎人");
	
	ImGui::Checkbox(XorStr("Allow Shove Jockey"), &m_bCanShoveJockey);
	IMGUI_TIPS("开启推猴子");
	
	ImGui::Checkbox(XorStr("Allow Shove Smoker"), &m_bCanShoveSmoker);
	IMGUI_TIPS("开启推舌头");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Handle Smoker"), &m_bSmoker);
	IMGUI_TIPS("处理舌头");

	ImGui::SliderFloat(XorStr("Smoker Distance"), &m_fSmokerDistance, 1.0f, 500.0f, ("%.1f"));
	IMGUI_TIPS("处理舌头范围");

	ImGui::Checkbox(XorStr("Handle Skeet"), &m_bHunter);
	IMGUI_TIPS("处理猎人");

	ImGui::SliderFloat(XorStr("Hunter Distance"), &m_fHunterDistance, 1.0f, 500.0f, ("%.1f"));
	IMGUI_TIPS("处理猎人范围");

	ImGui::Checkbox(XorStr("Handle Jockey"), &m_bJockey);
	IMGUI_TIPS("处理猴子");

	ImGui::SliderFloat(XorStr("Jockey Distance"), &m_fJockeyDistance, 1.0f, 500.0f, ("%.1f"));
	IMGUI_TIPS("处理猴子范围");

	ImGui::Checkbox(XorStr("Handle Charger"), &m_bCharger);
	IMGUI_TIPS("处理牛");

	ImGui::SliderFloat(XorStr("Charger Distance"), &m_fChargerDistance, 1.0f, 500.0f, ("%.1f"));
	IMGUI_TIPS("处理牛范围");

	ImGui::Checkbox(XorStr("Handle Witch"), &m_bWitch);
	IMGUI_TIPS("处理萌妹");

	ImGui::SliderFloat(XorStr("Witch Distance"), &m_fWitchDistance, 1.0f, 200.0f, ("%.1f"));
	IMGUI_TIPS("处理妹范围");

	ImGui::Checkbox(XorStr("Handle Boomer"), &m_bBoomer);
	IMGUI_TIPS("推胖子");

	ImGui::Checkbox(XorStr("Handle Rock"), &m_bRock);
	IMGUI_TIPS("处理坦克石头");

	ImGui::SliderFloat(XorStr("Handle Rock Distance"), &m_fRockDistance, 1.0f, 500.0f, ("%.1f"));
	IMGUI_TIPS("打石头范围");

	ImGui::Separator();
	ImGui::SliderFloat(XorStr("Hunter Lunge FOV"), &m_fHunterFov, 1.0f, 90.0f, ("%.1f"));
	IMGUI_TIPS("推猎人视野");

	ImGui::SliderFloat(XorStr("Jockey Lunge FOV"), &m_fJockeyFov, 1.0f, 90.0f, ("%.1f"));
	IMGUI_TIPS("推猴视野");

	ImGui::SliderFloat(XorStr("Jockey Scale"), &m_fJockeyScale, 0.0f, 5.0f, ("%.1f"), 0.1f);
	IMGUI_TIPS("猴子距离缩放");

	ImGui::SliderFloat(XorStr("Melee Scale"), &m_fMeleeScale, 0.0f, 5.0f, ("%.1f"), 0.1f);
	IMGUI_TIPS("近战距离缩放");

	ImGui::TreePop();
}

void CQuickTriggerEvent::OnConfigLoading(CProfile& cfg)
{
	const std::string mainKeys = XorStr("QuickTriggerEvent");

	m_bActive = cfg.GetBoolean(mainKeys, XorStr("qte_enable"), m_bActive);
	m_bVelExt = cfg.GetBoolean(mainKeys, XorStr("qte_velext"), m_bVelExt);
	m_bLagExt = cfg.GetBoolean(mainKeys, XorStr("qte_lagext"), m_bLagExt);
	m_bLagExt = cfg.GetBoolean(mainKeys, XorStr("qte_swingext"), m_bSwingExt);
	m_bSilent = cfg.GetBoolean(mainKeys, XorStr("qte_silent"), m_bSilent);
	m_bPerfectSilent = cfg.GetBoolean(mainKeys, XorStr("qte_psilent"), m_bPerfectSilent);
	m_bOnlyVisible = cfg.GetBoolean(mainKeys, XorStr("qte_visible"), m_bOnlyVisible);
	m_bCheckFov = cfg.GetBoolean(mainKeys, XorStr("qte_check_fov"), m_bCheckFov);
	m_bMeleeAsShove = cfg.GetBoolean(mainKeys, XorStr("qte_melee_tick"), m_bMeleeAsShove);
	m_iShoveTicks = cfg.GetInteger(mainKeys, XorStr("qte_shove_ticks"), m_iShoveTicks);
	m_bMeleeUnslienced = cfg.GetBoolean(mainKeys, XorStr("qte_melee_unslienced"), m_bMeleeUnslienced);
	m_fMeleeUnsliencedFov = cfg.GetFloat(mainKeys, XorStr("qte_melee_unslienced_fov"), m_fMeleeUnsliencedFov);

	m_bSmoker = cfg.GetBoolean(mainKeys, XorStr("qte_smoker"), m_bSmoker);
	m_bHunter = cfg.GetBoolean(mainKeys, XorStr("qte_hunter"), m_bHunter);
	m_bJockey = cfg.GetBoolean(mainKeys, XorStr("qte_jockey"), m_bJockey);
	m_bCharger = cfg.GetBoolean(mainKeys, XorStr("qte_charger"), m_bCharger);
	m_bWitch = cfg.GetBoolean(mainKeys, XorStr("qte_witch"), m_bWitch);
	m_bBoomer = cfg.GetBoolean(mainKeys, XorStr("qte_boomer"), m_bBoomer);
	m_bRock = cfg.GetBoolean(mainKeys, XorStr("qte_rock"), m_bRock);

	m_fHunterDistance = cfg.GetFloat(mainKeys, XorStr("qte_dst_hunter"), m_fHunterDistance);
	m_fJockeyDistance = cfg.GetFloat(mainKeys, XorStr("qte_dst_jockey"), m_fJockeyDistance);
	m_fChargerDistance = cfg.GetFloat(mainKeys, XorStr("qte_dst_charger"), m_fChargerDistance);
	m_fWitchDistance = cfg.GetFloat(mainKeys, XorStr("qte_dst_witch"), m_fWitchDistance);
	m_fRockDistance = cfg.GetFloat(mainKeys, XorStr("qte_dst_rock"), m_fRockDistance);
	m_fSmokerDistance = cfg.GetFloat(mainKeys, XorStr("qte_dst_smoker"), m_fSmokerDistance);

	m_fShoveDstExtra = cfg.GetFloat(mainKeys, XorStr("qte_dst_shove"), m_fShoveDstExtra);
	m_fMeleeDstExtra = cfg.GetFloat(mainKeys, XorStr("qte_dst_melee"), m_fMeleeDstExtra);

	m_fHunterFov = cfg.GetFloat(mainKeys, XorStr("qte_hunter_fov"), m_fHunterFov);
	m_fJockeyFov = cfg.GetFloat(mainKeys, XorStr("qte_jockey_fov"), m_fJockeyFov);
	m_fJockeyScale = cfg.GetFloat(mainKeys, XorStr("qte_jockey_scale"), m_fJockeyScale);
	m_fMeleeScale = cfg.GetFloat(mainKeys, XorStr("qte_melee_scale"), m_fMeleeScale);

	m_bCanShotSmoker = cfg.GetBoolean(mainKeys, XorStr("qte_smoker_selfclear"), m_bCanShotSmoker);
	m_bCanShotHunter = cfg.GetBoolean(mainKeys, XorStr("qte_hunter_skeet"), m_bCanShotHunter);
	m_bCanShotJockey = cfg.GetBoolean(mainKeys, XorStr("qte_jockey_skeet"), m_bCanShotJockey);
	m_bCanShotCharger = cfg.GetBoolean(mainKeys, XorStr("qte_charger_shot"), m_bCanShotCharger);
	m_bCanShotRock = cfg.GetBoolean(mainKeys, XorStr("qte_rock_skeet"), m_bCanShotRock);
	m_bCanMeleeSmoker = cfg.GetBoolean(mainKeys, XorStr("qte_smoker_tongue_cut"), m_bCanMeleeSmoker);
	m_bCanMeleeHunter = cfg.GetBoolean(mainKeys, XorStr("qte_hunter_skeet_cut"), m_bCanMeleeHunter);
	m_bCanMeleeJockey = cfg.GetBoolean(mainKeys, XorStr("qte_jockey_skeet_cut"), m_bCanMeleeJockey);
	m_bCanMeleeCharger = cfg.GetBoolean(mainKeys, XorStr("qte_charger_level"), m_bCanMeleeCharger);
	m_bCanMeleeRock = cfg.GetBoolean(mainKeys, XorStr("qte_rock_skeet_cut"), m_bCanMeleeRock);
	m_bCanShoveHunter = cfg.GetBoolean(mainKeys, XorStr("qte_hunter_deadstop"), m_bCanShoveHunter);
	m_bCanShoveJockey = cfg.GetBoolean(mainKeys, XorStr("qte_jockey_deadstop"), m_bCanShoveJockey);
	m_bCanShoveSmoker = cfg.GetBoolean(mainKeys, XorStr("qte_smoker_selfclear_shove"), m_bCanShoveSmoker);
}

void CQuickTriggerEvent::OnConfigSave(CProfile& cfg)
{
	const std::string mainKeys = XorStr("QuickTriggerEvent");

	cfg.SetValue(mainKeys, XorStr("qte_enable"), m_bActive);
	cfg.SetValue(mainKeys, XorStr("qte_velext"), m_bVelExt);
	cfg.SetValue(mainKeys, XorStr("qte_lagext"), m_bLagExt);
	cfg.SetValue(mainKeys, XorStr("qte_swingext"), m_bSwingExt);
	cfg.SetValue(mainKeys, XorStr("qte_silent"), m_bSilent);
	cfg.SetValue(mainKeys, XorStr("qte_psilent"), m_bPerfectSilent);
	cfg.SetValue(mainKeys, XorStr("qte_smoker"), m_bSmoker);
	cfg.SetValue(mainKeys, XorStr("qte_hunter"), m_bHunter);
	cfg.SetValue(mainKeys, XorStr("qte_jockey"), m_bJockey);
	cfg.SetValue(mainKeys, XorStr("qte_charger"), m_bCharger);
	cfg.SetValue(mainKeys, XorStr("qte_witch"), m_bWitch);
	cfg.SetValue(mainKeys, XorStr("qte_boomer"), m_bBoomer);
	cfg.SetValue(mainKeys, XorStr("qte_rock"), m_bRock);
	cfg.SetValue(mainKeys, XorStr("qte_dst_hunter"), m_fHunterDistance);
	cfg.SetValue(mainKeys, XorStr("qte_dst_jockey"), m_fJockeyDistance);
	cfg.SetValue(mainKeys, XorStr("qte_dst_charger"), m_fChargerDistance);
	cfg.SetValue(mainKeys, XorStr("qte_dst_smoker"), m_fSmokerDistance);
	cfg.SetValue(mainKeys, XorStr("qte_dst_witch"), m_fWitchDistance);
	cfg.SetValue(mainKeys, XorStr("qte_dst_rock"), m_fRockDistance);
	cfg.SetValue(mainKeys, XorStr("qte_dst_shove"), m_fShoveDstExtra);
	cfg.SetValue(mainKeys, XorStr("qte_dst_melee"), m_fMeleeDstExtra);
	cfg.SetValue(mainKeys, XorStr("qte_visible"), m_bOnlyVisible);
	cfg.SetValue(mainKeys, XorStr("qte_check_fov"), m_bCheckFov);
	cfg.SetValue(mainKeys, XorStr("qte_melee_tick"), m_bMeleeAsShove);
	cfg.SetValue(mainKeys, XorStr("qte_shove_ticks"), m_iShoveTicks);
	cfg.SetValue(mainKeys, XorStr("qte_hunter_fov"), m_fHunterFov);
	cfg.SetValue(mainKeys, XorStr("qte_jockey_fov"), m_fJockeyFov);
	cfg.SetValue(mainKeys, XorStr("qte_jockey_scale"), m_fJockeyScale);
	cfg.SetValue(mainKeys, XorStr("qte_melee_scale"), m_fMeleeScale);
	cfg.SetValue(mainKeys, XorStr("qte_melee_unslienced"), m_bMeleeUnslienced);
	cfg.SetValue(mainKeys, XorStr("qte_melee_unslienced_fov"), m_fMeleeUnsliencedFov);

	cfg.SetValue(mainKeys, XorStr("qte_smoker_selfclear"), m_bCanShotSmoker);
	cfg.SetValue(mainKeys, XorStr("qte_hunter_skeet"), m_bCanShotHunter);
	cfg.SetValue(mainKeys, XorStr("qte_jockey_skeet"), m_bCanShotJockey);
	cfg.SetValue(mainKeys, XorStr("qte_charger_shot"), m_bCanShotCharger);
	cfg.SetValue(mainKeys, XorStr("qte_rock_skeet"), m_bCanShotRock);
	cfg.SetValue(mainKeys, XorStr("qte_smoker_tongue_cut"), m_bCanMeleeSmoker);
	cfg.SetValue(mainKeys, XorStr("qte_hunter_skeet_cut"), m_bCanMeleeHunter);
	cfg.SetValue(mainKeys, XorStr("qte_jockey_skeet_cut"), m_bCanMeleeJockey);
	cfg.SetValue(mainKeys, XorStr("qte_charger_level"), m_bCanMeleeCharger);
	cfg.SetValue(mainKeys, XorStr("qte_rock_skeet_cut"), m_bCanMeleeRock);
	cfg.SetValue(mainKeys, XorStr("qte_hunter_deadstop"), m_bCanShoveHunter);
	cfg.SetValue(mainKeys, XorStr("qte_jockey_deadstop"), m_bCanShoveJockey);
	cfg.SetValue(mainKeys, XorStr("qte_smoker_selfclear_shove"), m_bCanShoveSmoker);
}

bool CQuickTriggerEvent::OnEmitSound(std::string& sample, int& entity, int& channel, float& volume, SoundLevel_t& level,
	int& flags, int& pitch, Vector& origin, Vector& direction, bool& updatePosition, float& soundTime)
{
	/*
	if (entity < 1 || entity > 64)
		return true;
	*/
	
	/*
	if (sample.find(XorStr("tongue")) != std::string::npos)
	{
		if (m_bLogInfo)
			g_pDrawing->PrintInfo(CDrawing::RED, XorStr("EmitSound %s at entity %d, org: %.0f, %.0f, %.0f"), sample.c_str(), entity, origin[0], origin[1], origin[2]);
		
		CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
		if (local == nullptr || local->GetTeam() != 2 || !local->IsAlive())
			return true;

		CBasePlayer* smoker = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(entity));
		if (smoker == nullptr || !smoker->IsAlive() || smoker->GetZombieType() != ZC_SMOKER)
			return true;
		
		Vector myOrigin = local->GetChestOrigin();
		Vector thatEyeOrigin = smoker->GetEyePosition();

		static ConVar* cvTongueRange = g_pInterface->Cvar->FindVar(XorStr("tongue_range"));
		if (myOrigin.DistTo(thatEyeOrigin) > cvTongueRange->GetFloat())
			return true;

		if (m_bCheckFov)
		{
			float fov = math::GetAnglesFieldOfView(smoker->GetEyeAngles(), math::CalculateAim(thatEyeOrigin, myOrigin));
			if (fov > 30.0f)
				return true;
		}

		m_pSmokerAttacker = smoker;
	}

	if (sample.find(XorStr("jockey_loudattack")) != std::string::npos ||
		sample.find(XorStr("smoker_launchtongue")) != std::string::npos ||
		sample.find(XorStr("smoker_tonguehit")) != std::string::npos ||
		sample.find(XorStr("hunter_attackmix")) != std::string::npos ||
		sample.find(XorStr("hunter_pounce")) != std::string::npos ||
		sample.find(XorStr("charger_charge")) != std::string::npos ||
		sample.find(XorStr("female_boomer_spotprey")) != std::string::npos ||
		sample.find(XorStr("male_boomer_spotprey")) != std::string::npos)
	{
		m_StartAttackTime[entity] = std::chrono::system_clock::now();
	}
	*/
	
	return true;
}

void CQuickTriggerEvent::HandleShotSelfClear(CBasePlayer * self,
	CBasePlayer * enemy, CUserCmd * cmd, float distance)
{
	QAngle aimAngles = GetAimAngles(self, enemy);
	if (aimAngles.IsValid())
	{
		cmd->buttons |= IN_ATTACK;
		SetAimAngles(cmd, aimAngles, false);
	}
}

void CQuickTriggerEvent::HandleMeleeSelfClear(CBasePlayer * self,
	CBasePlayer * enemy, CUserCmd * cmd, float distance, CBaseWeapon* weapon)
{
	static ConVar* cvMeleeRange = g_pInterface->Cvar->FindVar(XorStr("melee_range"));
	ZombieClass_t zClass = enemy->GetZombieType();
	
	// 砍舌头不看距离的
	if (zClass != ZC_SMOKER && distance > cvMeleeRange->GetFloat() + m_fMeleeDstExtra)
		return;

	QAngle aimAngles = GetAimAngles(self, enemy, false);
	if(!aimAngles.IsValid())
		return;
	
	cmd->buttons |= IN_ATTACK;
	
	/*
	if (zClass == ZC_SMOKER && enemy->GetSequence() == ANIM_SMOKER_SHOOTING)
		return;
	*/

	/*
	// 提升成功率
	if (zClass == ZC_CHARGER || zClass == ZC_SMOKER)
		aimAngles.x += 10.0f;
	else if(zClass == ZC_HUNTER || zClass == ZC_JOCKEY)
		aimAngles.x += 15.0f;
	*/
	
	if(zClass == ZC_SMOKER)
		aimAngles.x += 10.0f;
	else if(zClass == ZC_CHARGER)
		aimAngles.x = 9.15f;

	if(weapon == nullptr || !weapon->IsValid())
	{
		SetAimAngles(cmd, aimAngles, m_bMeleeAsShove, true);
		return;
	}
	
	if(weapon->CanFire())
	{
		// m_iMeleeState = MAS_PreAttack;
		m_pLastTarget = enemy;
		return;
	}
	
	/*
	auto now = std::chrono::system_clock::now();
	if(m_iMeleeState == MAS_PreAttack)
	{
		float delay = weapon->GetMeleeDelay();
		m_iMeleeState = MAS_Attacking;
		m_WaitTime = now + std::chrono::milliseconds(static_cast<long long>(delay * 1000));
		
		if(m_bLogInfo)
			g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("wait for %.1fs tick to start"), delay);
	}
	else if (m_iMeleeState == MAS_Attacking)
	{
		if (m_WaitTime <= now)
			m_iMeleeState = MAS_PostAttack;

		if (m_bLogInfo && m_iMeleeState == MAS_PostAttack)
			g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("start swing"));
	}
	if(m_iMeleeState == MAS_PostAttack)
	*/

	SetAimAngles(cmd, aimAngles, m_bMeleeAsShove, true);
}

void CQuickTriggerEvent::HandleShoveSelfClear(CBasePlayer * self,
	CBasePlayer * enemy, CUserCmd * cmd, float distance)
{
	static ConVar* cvShoveRange = g_pInterface->Cvar->FindVar(XorStr("z_gun_range"));
	if (distance > cvShoveRange->GetFloat() + m_fShoveDstExtra)
		return;

	QAngle aimAngles = GetAimAngles(self, enemy, false);
	if (aimAngles.IsValid() && self->CanShove())
	{
		cmd->buttons |= IN_ATTACK2;
		SetAimAngles(cmd, aimAngles, true);
	}
}

void CQuickTriggerEvent::HandleWitchCrown(CBasePlayer * self,
	CBasePlayer * enemy, CUserCmd * cmd, float distance)
{
	if (distance > 50.0f)
		return;

	Vector myEyeOrigin = self->GetEyePosition();
	Vector aimHeadOrigin = enemy->GetHitboxOrigin(HITBOX_WITCH_CHEST);
	if (m_bVelExt)
	{
		myEyeOrigin = math::VelocityExtrapolate(myEyeOrigin, self->GetVelocity(), m_bLagExt);
		// aimHeadOrigin = math::VelocityExtrapolate(aimHeadOrigin, enemy->GetVelocity(), m_bLagExt);
	}

	QAngle aimAngles = math::CalculateAim(myEyeOrigin, aimHeadOrigin);
	if (aimAngles.IsValid())
	{
		cmd->buttons |= IN_ATTACK;
		SetAimAngles(cmd, aimAngles);
	}
}

QAngle CQuickTriggerEvent::GetAimAngles(CBasePlayer * self, CBasePlayer * enemy, std::optional<bool> visable)
{
	Vector myEyeOrigin = self->GetEyePosition();
	Vector aimHeadOrigin = GetTargetAimPosition(enemy, visable);
	if (m_bVelExt)
	{
		myEyeOrigin = math::VelocityExtrapolate(myEyeOrigin, self->GetVelocity(), m_bLagExt);
		aimHeadOrigin = math::VelocityExtrapolate(aimHeadOrigin, enemy->GetVelocity(), m_bLagExt);
	}

	return math::CalculateAim(myEyeOrigin, aimHeadOrigin);
}

bool CQuickTriggerEvent::SetAimAngles(CUserCmd* cmd, QAngle& aimAngles, bool tick, bool melee)
{
	bool unslienced = (melee && m_bMeleeUnslienced);
	m_pLastTarget = nullptr;

	if (m_bPerfectSilent && !unslienced)
	{
		if(tick)
			return g_pViewManager->ApplySilentAngles(aimAngles, m_iShoveTicks);
		else
			return g_pViewManager->ApplySilentFire(aimAngles);
	}
	else if (m_bSilent && !unslienced)
		cmd->viewangles = aimAngles;
	else
		g_pInterface->Engine->SetViewAngles(aimAngles);
	
	return true;
}

bool CQuickTriggerEvent::IsVisibleEnemy(CBasePlayer * local, CBasePlayer * enemy, const Vector & start, const Vector & end)
{
	Ray_t ray;
	trace_t trace;
	CTraceFilter filter;

	ray.Init(start, end);
	filter.pSkip1 = local;

	try
	{
		g_pInterface->Trace->TraceRay(ray, MASK_SHOT, &filter, &trace);
	}
	catch (...)
	{
		Utils::logError(XorStr("CQuickTriggerEvent.IsVisibleEnemy.TraceRay Error."));
		return true;
	}

	return (trace.m_pEnt == enemy || trace.fraction > 0.97f);
}

bool CQuickTriggerEvent::HasShotgun(CBaseWeapon* weapon)
{
	if (weapon == nullptr)
		return false;

	int weaponId = weapon->GetWeaponID();
	return IsShotgun(weaponId);
}

Vector CQuickTriggerEvent::GetTargetAimPosition(CBasePlayer* entity, std::optional<bool> visible)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || entity == nullptr || !entity->IsValid())
		return NULL_VECTOR;

	if (entity->GetClassID() == ET_TankRock)
	{
		// 石头其实是 grenade 类的， traceray 无法命中它
		return entity->GetAbsOrigin();
	}

	if (!entity->IsAlive() || (entity->GetClassID() == ET_CTERRORPLAYER && entity->IsGhost()))
		return NULL_VECTOR;
	
	Vector startPosition = local->GetEyePosition();
	bool vis = visible.value_or(m_bOnlyVisible);
	bool chestFirst = HasShotgun(local->GetActiveWeapon());
	Vector aimPosition = (chestFirst ? entity->GetChestOrigin() : entity->GetHeadOrigin());
	if (!vis || IsVisibleEnemy(local, entity, startPosition, aimPosition))
		return aimPosition;

	aimPosition = (chestFirst ? entity->GetHeadOrigin() : entity->GetChestOrigin());
	if (!vis || IsVisibleEnemy(local, entity, startPosition, aimPosition))
		return aimPosition;

	return NULL_VECTOR;
}

CBasePlayer* CQuickTriggerEvent::FindTarget(CBasePlayer* local, const QAngle& myEyeAngles)
{
	float minDistance = 65535.0f;
	CBasePlayer* result = nullptr;
	Vector myEyePosition = local->GetEyePosition();
	int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();

	for (int i = 1; i <= maxEntity; ++i)
	{
		CBasePlayer* target = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(i));
		if (target == local || target == nullptr || !target->IsAlive())
			continue;

		if (target->IsPlayer() && target->IsGhost())
			continue;
		
		/*
		CBasePlayer* victim = target->GetCurrentTongueTarget();
		if (target->IsPlayer() && victim == local)
			return target;
		*/

		/*
		bool isAttacking = false;
		if (i <= 64)
		{
			auto now = std::chrono::system_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_StartAttackTime[i]).count();
			isAttacking = (duration <= 1000);
		}
		*/

		Vector aimPosition = GetTargetAimPosition(target);
		if (!aimPosition.IsValid())
			continue;

		ZombieClass_t classId = target->GetZombieType();
		float distance = myEyePosition.DistTo(aimPosition);
		int sequence = target->GetSequence();
		bool selected = false;

		switch (classId)
		{
			case ZC_SMOKER:
			{
				if (!m_bSmoker)
					break;

				static ConVar* cvTongueRange = g_pInterface->Cvar->FindVar(XorStr("tongue_range"));
				if (distance > cvTongueRange->GetFloat() || distance > m_fSmokerDistance)
					break;

				if (sequence != ANIM_SMOKER_PULLING && sequence != ANIM_SMOKER_SHOOTING /* && !isAttacking*/)
					break;

				float fov = math::GetAnglesFieldOfView(target->GetEyeAngles(), math::CalculateAim(target->GetEyePosition(), myEyePosition));
				if (m_bCheckFov && fov > 10.0f)
					break;

				selected = true;
				break;
			}
			case ZC_HUNTER:
			{
				if (!m_bHunter)
					break;

				if (!target->GetNetProp<BYTE>(XorStr("DT_TerrorPlayer"), XorStr("m_isAttemptingToPounce")) && sequence != ANIM_HUNTER_LUNGING)
					break;

				if ((target->GetFlags() & FL_ONGROUND)/* && !isAttacking*/)
					break;

				if (distance > m_fHunterDistance)
					break;

				float fov = math::GetAnglesFieldOfView(target->GetVelocity().toAngles(), math::CalculateAim(target->GetAbsOrigin(), myEyePosition));
				if (m_bCheckFov && fov > m_fHunterFov)
					break;

				selected = true;
				break;
			}
			case ZC_JOCKEY:
			{
				if (!m_bJockey)
					break;

				if ((target->GetFlags() & FL_ONGROUND)/* && !isAttacking*/)
					break;

				/*
				if (sequence == ANIM_JOCKEY_RIDEING)
					continue;
				*/

				/*
				if (sequence != ANIM_JOCKEY_LEAPING)
					continue;
				*/

				if (distance > m_fJockeyDistance)
					break;

				float fov = math::GetAnglesFieldOfView(target->GetVelocity().toAngles(), math::CalculateAim(target->GetAbsOrigin(), local->GetHeadOrigin()));
				if (m_bCheckFov && fov > m_fJockeyFov)
					break;

				selected = true;
				break;
			}
			case ZC_BOOMER:
			{
				if (!m_bBoomer)
					break;

				/*
				static ConVar* cvExplodeRange = g_pInterface->Cvar->FindVar(XorStr("z_exploding_splat_radius"));
				if (distance > cvExplodeRange->GetFloat())
					break;
				*/

				selected = true;
				break;
			}
			case ZC_CHARGER:
			{
				if (!m_bCharger)
					break;

				/*
				// 这个可能不正确
				handle = player->GetNetProp<CBaseHandle>(XorStr("DT_TerrorPlayer"), XorStr("m_customAbility"));
				if (handle.IsValid())
				{
					CBaseEntity* ability = reinterpret_cast<CBaseEntity*>(g_pInterface->EntList->GetClientEntityFromHandle(handle));
					if (ability != nullptr && !ability->GetNetProp<BYTE>(XorStr("DT_Charge"), XorStr("m_isCharging")))
						continue;
				}
				*/

				if (sequence != ANIM_CHARGER_CHARGING/* && !isAttacking*/)
					break;

				if (distance > m_fChargerDistance)
					break;

				float fov = math::GetAnglesFieldOfView(target->GetVelocity().toAngles(), math::CalculateAim(target->GetAbsOrigin(), myEyePosition));
				if (m_bCheckFov && fov > 30.0f)
					break;

				selected = true;
				break;
			}
			case ZC_WITCH:
			{
				if (!m_bWitch)
					break;

				if (target->GetNetProp<float>(XorStr("DT_Witch"), XorStr("m_rage")) < 1.0f)
					break;

				if (distance > m_fWitchDistance)
					break;

				/*
				CBaseEntity* entity = static_cast<CBaseEntity*>(player);
				fov = math::GetAnglesFieldOfView(entity->GetEyeAngles(), math::CalculateAim(entity->GetEyePosition(), myOrigin));
				if (m_bCheckFov && fov > 15.0f)
					continue;
				*/

				selected = true;
				break;
			}
			case ZC_ROCK:
			{
				if (!m_bRock)
					break;

				if (distance > m_fRockDistance)
					break;

				Vector velocity = target->GetVelocity();
				float fov = math::GetAnglesFieldOfView(velocity.toAngles(), math::CalculateAim(target->GetAbsOrigin(), myEyePosition));
				if (m_bCheckFov && fov > 15.0f)
					break;

				selected = true;
				break;
			}
		}

		if (selected && distance < minDistance)
		{
			result = target;
			minDistance = distance;
		}

		if (i > 64 && result)
			break;
	}

	return result;
}

std::unordered_set<CBasePlayer*> CQuickTriggerEvent::GetAllTongueSmoker()
{
	std::unordered_set<CBasePlayer*> results;
	
	int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();
	for (int i = 64; i <= maxEntity; ++i)
	{
		CBaseEntity* entity = reinterpret_cast<CBaseEntity*>(g_pInterface->EntList->GetClientEntity(i));
		if (!entity->IsValid() || entity->GetClassID() != ET_Particle)
			continue;

		CBasePlayer* owner = reinterpret_cast<CBasePlayer*>(entity->GetOwner());
		if (owner == nullptr || !owner->IsAlive() || owner->GetZombieType() != ZC_SMOKER)
			continue;

		results.insert(owner);
	}

	return results;
}
