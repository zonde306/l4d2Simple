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
#define ANIM_HUNTER_LUNGING			67
#define ANIM_JOCKEY_LEAPING			10
#define ANIM_JOCKEY_RIDEING			8

CQuickTriggerEvent::CQuickTriggerEvent() : CBaseFeatures::CBaseFeatures()
{
	for (int i = 0; i < 65; ++i)
		m_StartAttackTime[i] = std::chrono::system_clock::from_time_t(0);
}

CQuickTriggerEvent::~CQuickTriggerEvent()
{
	CBaseFeatures::~CBaseFeatures();
}

void CQuickTriggerEvent::OnCreateMove(CUserCmd * cmd, bool*)
{
	if (!m_bActive)
		return;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || local->GetTeam() != 2 || !local->IsAlive() ||
		local->IsIncapacitated() || local->IsHangingFromLedge() ||
		local->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMountedGun")) ||
		local->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMountedWeapon")))
		return;

	CBasePlayer* player = m_pSmokerAttacker;
	m_pSmokerAttacker = nullptr;

	CBasePlayer* dominater = local->GetCurrentAttacker();
	if (dominater != nullptr && dominater->IsValid() && dominater->IsAlive() && dominater != player)
		return;

	CBaseWeapon* weapon = local->GetActiveWeapon();
	if (weapon == nullptr)
		return;

	float distance = 1000.0f;
	int weaponId = weapon->GetWeaponID();
	bool canShot = (m_bForceShot && weapon->IsFireGun() && weapon->CanFire());
	bool canFire = (m_bAllowShot && weapon->IsFireGun() && weapon->CanFire());
	bool hasMelee = (m_bAllowMelee && weaponId == Weapon_Melee);
	bool canShove = weapon->CanShove();
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
				HandleMeleeSelfClear(local, player, cmd, distance);
			else if (canFire)
				HandleShotSelfClear(local, player, cmd, distance);
			else
				HandleShoveSelfClear(local, player, cmd, distance);

			// 优先处理被拉自救，其他的先不管了
			return;
		}
	}
	*/

	if (m_bVelExt)
		myOrigin = math::VelocityExtrapolate(myOrigin, local->GetVelocity(), m_bLagExt);

	if (player == nullptr)
		player = FindTarget(local, cmd->viewangles);
	if (player == nullptr)
		return;

	distance = myOrigin.DistTo(player->GetAbsOrigin());
	ZombieClass_t classId = player->GetZombieType();

	switch (classId)
	{
		case ZC_SMOKER:
		case ZC_HUNTER:
		case ZC_JOCKEY:
		{
			if (hasMelee)
			{
				// 近战必须提前，否则砍不到的
				HandleMeleeSelfClear(local, player, cmd, (classId == ZC_JOCKEY ? distance / m_fJockeyScale / m_fMeleeScale : distance / m_fMeleeScale));
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("melee attack %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			else if (canFire)
			{
				HandleShotSelfClear(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("gun shot %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}
			else if (canShove)
			{
				// 推猴子必须提前
				HandleShoveSelfClear(local, player, cmd, (classId == ZC_JOCKEY ? distance / m_fJockeyScale : distance));
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("shove %s, dist: %.0f"), player->GetCharacterName().c_str(), distance);
			}

			break;
		}
		case ZC_CHARGER:
		case ZC_ROCK:
		{
			if (hasMelee)
			{
				// 近战必须提前，否则砍不到的
				HandleMeleeSelfClear(local, player, cmd, distance / m_fMeleeScale);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("melee attack charger/rock(%d), dist: %.0f"), classId, distance);
			}
			else if (canFire)
			{
				HandleShotSelfClear(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("gun shot charger/rock(%d), dist: %.0f"), classId, distance);
			}

			break;
		}
		case ZC_WITCH:
		{
			if ((canFire || canShot) && IsShotgun(weaponId))
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
			if (canShove)
			{
				HandleShoveSelfClear(local, player, cmd, distance);
				if (m_bLogInfo)
					g_pDrawing->PrintInfo(CDrawing::WHITE, XorStr("shove boomer/spitter(%d), dist: %.0f"), classId, distance);
			}
			break;
		}
	}
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

	ImGui::Checkbox(XorStr("Silent Aim"), &m_bSilent);
	IMGUI_TIPS("自己看不到自动瞄准，建议开启");

	ImGui::Checkbox(XorStr("Perfect Silent"), &m_bPerfectSilent);
	IMGUI_TIPS("观察者看不到自动瞄准，建议开启");

	ImGui::Checkbox(XorStr("Visible Only"), &m_bOnlyVisible);
	IMGUI_TIPS("可见检查");
	
	ImGui::Checkbox(XorStr("Target Check"), &m_bCheckFov);
	IMGUI_TIPS("目标检查");
	
	ImGui::Checkbox(XorStr("Allow Shot"), &m_bAllowShot);
	IMGUI_TIPS("允许射击");
	
	ImGui::Checkbox(XorStr("Allow Shot Witch"), &m_bForceShot);
	IMGUI_TIPS("允许射击 Witch");
	
	ImGui::Checkbox(XorStr("Allow Melee"), &m_bAllowMelee);
	IMGUI_TIPS("允许近战");
	
	ImGui::Checkbox(XorStr("Melee TickMode"), &m_bMeleeAsShove);
	IMGUI_TIPS("近战武器攻击使用推的形式(基于tick)，如果近战砍不到则开启");
	
	ImGui::Checkbox(XorStr("Show Info"), &m_bLogInfo);
	IMGUI_TIPS("显示信息");

	ImGui::SliderFloat(XorStr("Shove Range Extra"), &m_fShoveDstExtra, 1.0f, 300.0f, ("%.1f"));
	IMGUI_TIPS("推预测范围");

	ImGui::SliderFloat(XorStr("Melee Range Extra"), &m_fMeleeDstExtra, 1.0f, 300.0f, ("%.1f"));
	IMGUI_TIPS("近战预测范围");
	
	ImGui::SliderInt(XorStr("Shove Ticks"), &m_iShoveTicks, 1, 15);
	IMGUI_TIPS("推 tick 数量");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Smoker SelfClear"), &m_bSmoker);
	IMGUI_TIPS("被拉自救");

	ImGui::SliderFloat(XorStr("SmokerSelfClear Distance"), &m_fSmokerDistance, 1.0f, 500.0f, ("%.1f"));
	IMGUI_TIPS("被拉自救范围");

	ImGui::Checkbox(XorStr("Hunter Skeet"), &m_bHunter);
	IMGUI_TIPS("空爆猎人");

	ImGui::SliderFloat(XorStr("HunterSkeet Distance"), &m_fHunterDistance, 1.0f, 500.0f, ("%.1f"));
	IMGUI_TIPS("空爆猎人范围");

	ImGui::Checkbox(XorStr("Jockey Skeet"), &m_bJockey);
	IMGUI_TIPS("推猴子");

	ImGui::SliderFloat(XorStr("JockeySkeet Distance"), &m_fJockeyDistance, 1.0f, 500.0f, ("%.1f"));
	IMGUI_TIPS("推猴范围");

	ImGui::Checkbox(XorStr("Charge Level"), &m_bCharger);
	IMGUI_TIPS("近战秒牛");

	ImGui::SliderFloat(XorStr("ChargeLevel Distance"), &m_fChargerDistance, 1.0f, 500.0f, ("%.1f"));
	IMGUI_TIPS("秒牛范围");

	ImGui::Checkbox(XorStr("Witch Crown"), &m_bWitch);
	IMGUI_TIPS("引秒萌妹");

	ImGui::SliderFloat(XorStr("WitchCrown Distance"), &m_fWitchDistance, 1.0f, 200.0f, ("%.1f"));
	IMGUI_TIPS("秒妹范围");

	ImGui::Checkbox(XorStr("BoomerPop"), &m_bBoomer);
	IMGUI_TIPS("推胖子");

	ImGui::Checkbox(XorStr("Rock Skeet"), &m_bRock);
	IMGUI_TIPS("反坦克石头");

	ImGui::SliderFloat(XorStr("RockSkeet Distance"), &m_fRockDistance, 1.0f, 500.0f, ("%.1f"));
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

void CQuickTriggerEvent::OnConfigLoading(const config_type & data)
{
	const std::string mainKeys = XorStr("QuickTriggerEvent");

	m_bActive = g_pConfig->GetBoolean(mainKeys, XorStr("qte_enable"), m_bActive);
	m_bVelExt = g_pConfig->GetBoolean(mainKeys, XorStr("qte_velext"), m_bVelExt);
	m_bLagExt = g_pConfig->GetBoolean(mainKeys, XorStr("qte_lagext"), m_bLagExt);
	m_bSilent = g_pConfig->GetBoolean(mainKeys, XorStr("qte_silent"), m_bSilent);
	m_bPerfectSilent = g_pConfig->GetBoolean(mainKeys, XorStr("qte_psilent"), m_bPerfectSilent);
	m_bOnlyVisible = g_pConfig->GetBoolean(mainKeys, XorStr("qte_visible"), m_bOnlyVisible);
	m_bAllowShot = g_pConfig->GetBoolean(mainKeys, XorStr("qte_shot"), m_bAllowShot);
	m_bForceShot = g_pConfig->GetBoolean(mainKeys, XorStr("qte_shot_forced"), m_bForceShot);
	m_bAllowMelee = g_pConfig->GetBoolean(mainKeys, XorStr("qte_melee"), m_bAllowMelee);
	m_bCheckFov = g_pConfig->GetBoolean(mainKeys, XorStr("qte_check_fov"), m_bCheckFov);
	m_bMeleeAsShove = g_pConfig->GetBoolean(mainKeys, XorStr("qte_melee_tick"), m_bMeleeAsShove);
	m_iShoveTicks = g_pConfig->GetInteger(mainKeys, XorStr("qte_shove_ticks"), m_iShoveTicks);

	m_bSmoker = g_pConfig->GetBoolean(mainKeys, XorStr("qte_smoker"), m_bSmoker);
	m_bHunter = g_pConfig->GetBoolean(mainKeys, XorStr("qte_hunter"), m_bHunter);
	m_bJockey = g_pConfig->GetBoolean(mainKeys, XorStr("qte_jockey"), m_bJockey);
	m_bCharger = g_pConfig->GetBoolean(mainKeys, XorStr("qte_charger"), m_bCharger);
	m_bWitch = g_pConfig->GetBoolean(mainKeys, XorStr("qte_witch"), m_bWitch);
	m_bBoomer = g_pConfig->GetBoolean(mainKeys, XorStr("qte_boomer"), m_bBoomer);
	m_bRock = g_pConfig->GetBoolean(mainKeys, XorStr("qte_rock"), m_bRock);

	m_fHunterDistance = g_pConfig->GetFloat(mainKeys, XorStr("qte_dst_hunter"), m_fHunterDistance);
	m_fJockeyDistance = g_pConfig->GetFloat(mainKeys, XorStr("qte_dst_jockey"), m_fJockeyDistance);
	m_fChargerDistance = g_pConfig->GetFloat(mainKeys, XorStr("qte_dst_charger"), m_fChargerDistance);
	m_fWitchDistance = g_pConfig->GetFloat(mainKeys, XorStr("qte_dst_witch"), m_fWitchDistance);
	m_fRockDistance = g_pConfig->GetFloat(mainKeys, XorStr("qte_dst_rock"), m_fRockDistance);
	m_fSmokerDistance = g_pConfig->GetFloat(mainKeys, XorStr("qte_dst_smoker"), m_fSmokerDistance);

	m_fShoveDstExtra = g_pConfig->GetFloat(mainKeys, XorStr("qte_dst_shove"), m_fShoveDstExtra);
	m_fMeleeDstExtra = g_pConfig->GetFloat(mainKeys, XorStr("qte_dst_melee"), m_fMeleeDstExtra);

	m_fHunterFov = g_pConfig->GetFloat(mainKeys, XorStr("qte_hunter_fov"), m_fHunterFov);
	m_fJockeyFov = g_pConfig->GetFloat(mainKeys, XorStr("qte_jockey_fov"), m_fJockeyFov);
	m_fJockeyScale = g_pConfig->GetFloat(mainKeys, XorStr("qte_jockey_scale"), m_fJockeyScale);
	m_fMeleeScale = g_pConfig->GetFloat(mainKeys, XorStr("qte_melee_scale"), m_fMeleeScale);
}

void CQuickTriggerEvent::OnConfigSave(config_type & data)
{
	const std::string mainKeys = XorStr("QuickTriggerEvent");

	g_pConfig->SetValue(mainKeys, XorStr("qte_enable"), m_bActive);
	g_pConfig->SetValue(mainKeys, XorStr("qte_velext"), m_bVelExt);
	g_pConfig->SetValue(mainKeys, XorStr("qte_lagext"), m_bLagExt);
	g_pConfig->SetValue(mainKeys, XorStr("qte_silent"), m_bSilent);
	g_pConfig->SetValue(mainKeys, XorStr("qte_psilent"), m_bPerfectSilent);
	g_pConfig->SetValue(mainKeys, XorStr("qte_smoker"), m_bSmoker);
	g_pConfig->SetValue(mainKeys, XorStr("qte_hunter"), m_bHunter);
	g_pConfig->SetValue(mainKeys, XorStr("qte_jockey"), m_bJockey);
	g_pConfig->SetValue(mainKeys, XorStr("qte_charger"), m_bCharger);
	g_pConfig->SetValue(mainKeys, XorStr("qte_witch"), m_bWitch);
	g_pConfig->SetValue(mainKeys, XorStr("qte_boomer"), m_bBoomer);
	g_pConfig->SetValue(mainKeys, XorStr("qte_rock"), m_bRock);
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_hunter"), m_fHunterDistance);
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_jockey"), m_fJockeyDistance);
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_charger"), m_fChargerDistance);
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_smoker"), m_fSmokerDistance);
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_witch"), m_fWitchDistance);
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_rock"), m_fRockDistance);
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_shove"), m_fShoveDstExtra);
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_melee"), m_fMeleeDstExtra);
	g_pConfig->SetValue(mainKeys, XorStr("qte_visible"), m_bOnlyVisible);
	g_pConfig->SetValue(mainKeys, XorStr("qte_shot"), m_bAllowShot);
	g_pConfig->SetValue(mainKeys, XorStr("qte_shot_forced"), m_bForceShot);
	g_pConfig->SetValue(mainKeys, XorStr("qte_melee"), m_bAllowMelee);
	g_pConfig->SetValue(mainKeys, XorStr("qte_check_fov"), m_bCheckFov);
	g_pConfig->SetValue(mainKeys, XorStr("qte_melee_tick"), m_bMeleeAsShove);
	g_pConfig->SetValue(mainKeys, XorStr("qte_shove_ticks"), m_iShoveTicks);
	g_pConfig->SetValue(mainKeys, XorStr("qte_hunter_fov"), m_fHunterFov);
	g_pConfig->SetValue(mainKeys, XorStr("qte_jockey_fov"), m_fJockeyFov);
	g_pConfig->SetValue(mainKeys, XorStr("qte_jockey_scale"), m_fJockeyScale);
	g_pConfig->SetValue(mainKeys, XorStr("qte_melee_scale"), m_fMeleeScale);
}

bool CQuickTriggerEvent::OnEmitSound(std::string& sample, int& entity, int& channel, float& volume, SoundLevel_t& level,
	int& flags, int& pitch, Vector& origin, Vector& direction, bool& updatePosition, float& soundTime)
{
	if (entity < 1 || entity > 64)
		return true;
	
	if (sample.find(XorStr("smoker_launchtongue")) != std::string::npos ||
		sample.find(XorStr("smoker_tonguehit")) != std::string::npos)
	{
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

		if(m_bLogInfo)
			g_pDrawing->PrintInfo(CDrawing::RED, XorStr("somker(%s) attack warning"), smoker->GetName().c_str());
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
	
	return true;
}

void CQuickTriggerEvent::HandleShotSelfClear(CBasePlayer * self,
	CBasePlayer * enemy, CUserCmd * cmd, float distance)
{
	QAngle aimAngles = GetAimAngles(self, enemy);
	if (aimAngles.IsValid())
	{
		cmd->buttons |= IN_ATTACK;
		SetAimAngles(cmd, aimAngles);
	}
}

void CQuickTriggerEvent::HandleMeleeSelfClear(CBasePlayer * self,
	CBasePlayer * enemy, CUserCmd * cmd, float distance)
{
	static ConVar* cvMeleeRange = g_pInterface->Cvar->FindVar(XorStr("melee_range"));
	if (distance > cvMeleeRange->GetFloat() + m_fMeleeDstExtra)
		return;

	QAngle aimAngles = GetAimAngles(self, enemy, false);
	if (aimAngles.IsValid())
	{
		cmd->buttons |= IN_ATTACK;

		/*
		// 砍舌头需要稍微往下一点，不然砍不到
		if (enemy->GetZombieType() == ZC_SMOKER)
			aimAngles.x += 15.0f;
		*/

		SetAimAngles(cmd, aimAngles, m_bMeleeAsShove);
	}
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

void CQuickTriggerEvent::SetAimAngles(CUserCmd* cmd, QAngle& aimAngles, bool tick)
{
	if (m_bPerfectSilent)
	{
		if(tick)
			g_pViewManager->ApplySilentAngles(aimAngles, m_iShoveTicks);
		else
			g_pViewManager->ApplySilentFire(aimAngles);
	}
	else if (m_bSilent)
		cmd->viewangles = aimAngles;
	else

		g_pInterface->Engine->SetViewAngles(aimAngles);
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

		bool isAttacking = false;
		if (i <= 64)
		{
			auto now = std::chrono::system_clock::now();
			auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_StartAttackTime[i]).count();
			isAttacking = (duration <= 1000);
		}

		Vector aimPosition = GetTargetAimPosition(target);
		if (!aimPosition.IsValid())
			continue;

		ZombieClass_t classId = target->GetZombieType();
		float distance = myEyePosition.DistTo(target->GetAbsOrigin());
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

				if (target->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSequence")) != ANIM_SMOKER_PULLING && !isAttacking)
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

				if (!target->GetNetProp<BYTE>(XorStr("DT_TerrorPlayer"), XorStr("m_isAttemptingToPounce")) &&
					target->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSequence")) != ANIM_HUNTER_LUNGING)
					break;

				if ((target->GetFlags() & FL_ONGROUND) && !isAttacking)
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

				if ((target->GetFlags() & FL_ONGROUND) && !isAttacking)
					break;

				/*
				if (player->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSequence")) == ANIM_JOCKEY_RIDEING)
					continue;
				*/

				/*
				if (player->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSequence")) != ANIM_JOCKEY_LEAPING)
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

				static ConVar* cvExplodeRange = g_pInterface->Cvar->FindVar(XorStr("z_exploding_splat_radius"));
				if (distance > cvExplodeRange->GetFloat())
					break;

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

				if (target->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSequence")) != ANIM_CHARGER_CHARGING && !isAttacking)
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

				const Vector& velocity = target->GetNetProp<Vector>(XorStr("DT_BaseGrenade"), XorStr("m_vecVelocity"));
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
