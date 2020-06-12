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
	if (local == nullptr || local->GetTeam() != 2 || !local->IsAlive())
		return;

	CBaseWeapon* weapon = local->GetActiveWeapon();
	if (weapon == nullptr)
		return;

	float distance = 1000.0f;
	CBasePlayer* player = nullptr;
	bool canFire = (m_bAllowShot && weapon->IsFireGun() && weapon->CanFire());
	bool hasMelee = (weapon->GetWeaponID() == Weapon_Melee);

	/*
	// 这里没有效果。。。
	CBaseHandle handle = local->GetNetProp<CBaseHandle>(XorStr("DT_TerrorPlayer"), XorStr("m_tongueOwner"));
	if (handle.IsValid() && !local->GetNetProp<BYTE>(XorStr("DT_TerrorPlayer"), XorStr("m_isProneTongueDrag")))
	{
		player = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntityFromHandle(handle));
		if (player != nullptr)
		{
			distance = local->GetAbsOrigin().DistTo(player->GetAbsOrigin());
			
			if (hasMelee)
				HandleMeleeSelfClear(local, player, cmd, distance);
			else if (canFire)
				HandleShotSelfClear(local, player, cmd, distance);
			else
				HandleShoveSelfClear(local, player, cmd, distance);

			return;
		}
	}
	*/

	Vector myOrigin = local->GetEyePosition();

	if (m_pSmokerAttacker && m_pSmokerAttacker->IsAlive())
	{
		float distance = myOrigin.DistTo(myOrigin);

		if (hasMelee)
			HandleMeleeSelfClear(local, m_pSmokerAttacker, cmd, distance);
		else if (canFire)
			HandleShotSelfClear(local, m_pSmokerAttacker, cmd, distance);
		else
			HandleShoveSelfClear(local, m_pSmokerAttacker, cmd, distance);

		// 优先处理被拉自救，其他的先不管了
		return;
	}

	m_pSmokerAttacker = nullptr;

	if (m_bVelExt)
		myOrigin = math::VelocityExtrapolate(myOrigin, local->GetVelocity(), m_bLagExt);

	float fov = 360.0f;
	Vector aimOrigin;
	int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();
	for (int i = 1; i <= maxEntity; ++i)
	{
		player = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(i));
		if (player == nullptr || player == local)
			continue;

		aimOrigin = GetTargetAimPosition(player);
		if (!aimOrigin.IsValid())
			continue;

		if (m_bVelExt)
			aimOrigin = math::VelocityExtrapolate(aimOrigin, player->GetVelocity(), m_bLagExt);

		ZombieClass_t classId = player->GetZombieType();
		distance = myOrigin.DistTo(aimOrigin);
		fov = math::GetAnglesFieldOfView(player->GetEyeAngles(), math::CalculateAim(player->GetEyePosition(), myOrigin));

		/*
		if (classId != ZC_SMOKER && classId != ZC_HUNTER && classId != ZC_JOCKEY && classId != ZC_CHARGER)
			continue;
		*/

		switch (classId)
		{
			case ZC_SMOKER:
			{
				if (!m_bSmoker)
					continue;
				
				static ConVar* cvTongueRange = g_pInterface->Cvar->FindVar(XorStr("tongue_range"));
				if (distance > cvTongueRange->GetFloat())
					continue;

				if (player->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSequence")) != ANIM_SMOKER_PULLING)
					continue;

				if (m_bCheckFov && fov > 30.0f)
					continue;

				break;
			}
			case ZC_HUNTER:
			{
				if (!m_bHunter)
					continue;
				
				if (!player->GetNetProp<BYTE>(XorStr("DT_TerrorPlayer"), XorStr("m_isAttemptingToPounce")) &&
					player->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSequence")) != ANIM_HUNTER_LUNGING)
					continue;

				if (player->GetFlags() & FL_ONGROUND)
					continue;
				
				if (distance > m_fHunterDistance)
					continue;

				if (m_bCheckFov && fov > 15.0f)
					continue;

				break;
			}
			case ZC_JOCKEY:
			{
				if (!m_bJockey)
					continue;
				
				if (player->GetFlags() & FL_ONGROUND)
					continue;
				
				if (player->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSequence")) == ANIM_JOCKEY_RIDEING)
					continue;

				/*
				if (player->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSequence")) != ANIM_JOCKEY_LEAPING)
					continue;
				*/

				if (distance > m_fJockeyDistance)
					continue;

				if (m_bCheckFov && fov > 15.0f)
					continue;

				break;
			}
			case ZC_BOOMER:
			{
				if (!m_bBoomer)
					continue;
				
				static ConVar* cvExplodeRange = g_pInterface->Cvar->FindVar(XorStr("z_exploding_splat_radius"));
				if (distance > cvExplodeRange->GetFloat())
					continue;

				break;
			}
			case ZC_CHARGER:
			{
				if (!m_bCharger)
					continue;
				
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

				if (player->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSequence")) != ANIM_CHARGER_CHARGING)
					continue;

				if (distance > m_fChargerDistance)
					continue;

				if (m_bCheckFov && fov > 30.0f)
					continue;

				break;
			}
			case ZC_WITCH:
			{
				if (!m_bWitch)
					continue;
				
				if (player->GetNetProp<float>(XorStr("DT_Witch"), XorStr("m_rage")) < 1.0f)
					continue;

				if (distance > m_fWitchDistance)
					continue;

				if (m_bCheckFov && fov > 15.0f)
					continue;

				break;
			}
			case ZC_ROCK:
			{
				if (!m_bRock)
					continue;
				
				if (distance > m_fRockDistance)
					continue;

				if (m_bCheckFov && fov > 15.0f)
					continue;

				break;
			}
		}

		switch (classId)
		{
			case ZC_SMOKER:
			case ZC_HUNTER:
			case ZC_JOCKEY:
			{
				if (hasMelee)
					HandleMeleeSelfClear(local, player, cmd, distance);
				else if (canFire)
					HandleShotSelfClear(local, player, cmd, distance);
				else
					HandleShoveSelfClear(local, player, cmd, distance);

				break;
			}
			case ZC_CHARGER:
			case ZC_ROCK:
			{
				if (hasMelee)
					HandleMeleeSelfClear(local, player, cmd, distance);
				else if (canFire)
					HandleShotSelfClear(local, player, cmd, distance);
				
				break;
			}
			case ZC_WITCH:
			{
				int weaponId = weapon->GetWeaponID();
				if (canFire && IsShotgun(weaponId))
					HandleWitchCrown(local, player, cmd, distance);

				break;
			}
			case ZC_BOOMER:
			case ZC_SPITTER:
			{
				HandleShoveSelfClear(local, player, cmd, distance);
				break;
			}
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

	ImGui::SliderFloat(XorStr("Shove Range Extra"), &m_fShoveDstExtra, 1.0f, 300.0f, ("%.1f"));
	IMGUI_TIPS("推预测范围");

	ImGui::SliderFloat(XorStr("Melee Range Extra"), &m_fMeleeDstExtra, 1.0f, 300.0f, ("%.1f"));
	IMGUI_TIPS("近战预测范围");
	
	ImGui::SliderFloat(XorStr("Shove Ticks"), &m_iShoveTicks, 1.0f, 15.0f, ("%.0f"), 1.0f);
	IMGUI_TIPS("推 tick 数量");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Smoker SelfClear"), &m_bSmoker);
	IMGUI_TIPS("被拉自救");

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
	m_bCheckFov = g_pConfig->GetBoolean(mainKeys, XorStr("qte_check_fov"), m_bCheckFov);
	m_iShoveTicks = g_pConfig->GetFloat(mainKeys, XorStr("qte_shove_ticks"), m_iShoveTicks);

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

	m_fShoveDstExtra = g_pConfig->GetFloat(mainKeys, XorStr("qte_dst_shove"), m_fShoveDstExtra);
	m_fMeleeDstExtra = g_pConfig->GetFloat(mainKeys, XorStr("qte_dst_melee"), m_fMeleeDstExtra);
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
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_witch"), m_fWitchDistance);
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_rock"), m_fRockDistance);
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_shove"), m_fShoveDstExtra);
	g_pConfig->SetValue(mainKeys, XorStr("qte_dst_melee"), m_fMeleeDstExtra);
	g_pConfig->SetValue(mainKeys, XorStr("qte_visible"), m_bOnlyVisible);
	g_pConfig->SetValue(mainKeys, XorStr("qte_shot"), m_bAllowShot);
	g_pConfig->SetValue(mainKeys, XorStr("qte_check_fov"), m_bCheckFov);
	g_pConfig->SetValue(mainKeys, XorStr("qte_shove_ticks"), m_iShoveTicks);
}

bool CQuickTriggerEvent::OnEmitSound(std::string& sample, int& entity, int& channel, float& volume, SoundLevel_t& level,
	int& flags, int& pitch, Vector& origin, Vector& direction, bool& updatePosition, float& soundTime)
{
	if (entity > 0 && entity < 64 &&
		(sample.find(XorStr("smoker_launchtongue")) != std::string::npos ||
			sample.find(XorStr("smoker_tonguehit")) != std::string::npos))
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

		// 砍舌头需要稍微往下一点，不然砍不到
		if (enemy->GetZombieType() == ZC_SMOKER)
			aimAngles.x += 10.0f;

		SetAimAngles(cmd, aimAngles);
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
		aimHeadOrigin = math::VelocityExtrapolate(aimHeadOrigin, enemy->GetVelocity(), m_bLagExt);
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
			g_pViewManager->ApplySilentAngles(aimAngles, static_cast<int>(m_iShoveTicks));
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
