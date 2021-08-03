#include "Knifebot.h"
#include "../Utils/math.h"
#include "../hook.h"
#include "../../l4d2Simple2/config.h"

CKnifeBot* g_pKnifeBot = nullptr;

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
#define ANIM_GRENADE_THORWING	378
#define ANIM_GRENADE_THROWED	373

CKnifeBot::CKnifeBot() : CBaseFeatures::CBaseFeatures()
{
}

CKnifeBot::~CKnifeBot()
{
	CBaseFeatures::~CBaseFeatures();
}

void CKnifeBot::OnCreateMove(CUserCmd * cmd, bool *)
{
	// 防止打断救人
	if ((cmd->buttons & IN_USE) || m_bMenuOpen)
		return;
	
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive() || player->IsGhost() ||
		player->IsIncapacitated() || player->IsHangingFromLedge() ||
		player->GetCurrentAttacker() != nullptr ||
		player->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMountedGun")) ||
		player->GetNetProp<byte>(XorStr("DT_TerrorPlayer"), XorStr("m_usingMountedWeapon")))
		return;

	CBaseWeapon* weapon = player->GetActiveWeapon();
	if (weapon == nullptr || !weapon->IsValid() ||
		weapon->GetNetProp<float>(XorStr("DT_BaseAnimating"), XorStr("m_flCycle")) > 0.0f)
		return;
	
	int team = player->GetTeam();
	if (team == 3 && player->IsGhost())
		return;

	int weaponId = weapon->GetWeaponID();
	float nextAttack = weapon->GetNextPrimaryAttack();
	float serverTime = g_pClientPrediction->GetServerTime();

	if (m_bFastMelee && team == 2 && (cmd->buttons & IN_RELOAD))
	{
		RunFastMelee(cmd, weaponId, nextAttack, serverTime);
		return;
	}

	if (cmd->buttons & IN_ATTACK)
		return;

	QAngle viewAngles;
	g_pInterface->Engine->GetViewAngles(viewAngles);
	CheckMeleeAttack(viewAngles);

	bool isTank = (player->GetZombieType() == ZC_TANK);
	bool isMelee = (team == 2 && weaponId == Weapon_Melee);

	if (m_bAutoFire && m_bCanMeleeAttack)
	{
		if ((isMelee || isTank) && nextAttack <= serverTime)
		{
			cmd->buttons |= IN_ATTACK;
			return;
		}
	}

	if (m_bAutoShove && m_bCanShoveAttack && IsShoveReady(player, weapon))
	{
		if (!isTank && player->CanShove())
		{
			cmd->buttons |= IN_ATTACK2;
			return;
		}
	}
}

void CKnifeBot::OnMenuDrawing()
{
	if (!ImGui::TreeNode(XorStr("KnifeBot")))
		return;

	ImGui::Checkbox(XorStr("Auto Melee"), &m_bAutoFire);
	IMGUI_TIPS("手持近战武器时自动攻击敌人。");

	ImGui::Checkbox(XorStr("Auto Shove"), &m_bAutoShove);
	IMGUI_TIPS("当敌人接近时自动推开/抓。");

	ImGui::Checkbox(XorStr("Melee Faster"), &m_bFastMelee);
	IMGUI_TIPS("近战武器速砍，按住 R 启动。");

	ImGui::Checkbox(XorStr("Visual Only"), &m_bVisualOnly);
	IMGUI_TIPS("只有目标看得见才触发");

	ImGui::Separator();
	ImGui::Checkbox(XorStr("Melee Velocity Extrapolate"), &m_bVelExt);
	IMGUI_TIPS("速度预测，可以提升精度。");

	ImGui::Checkbox(XorStr("Melee Forwardtrack"), &m_bForwardtrack);
	IMGUI_TIPS("速度延迟预测，需要开启上面的才能用。");

	ImGui::Separator();
	ImGui::SliderFloat(XorStr("Auto Melee FOV"), &m_fMeleeFOV, 10.0f, 120.0f, XorStr("%.0f"));
	IMGUI_TIPS("近战武器攻击角度。");

	ImGui::SliderFloat(XorStr("Auto Shove FOV"), &m_fShoveFOV, 10.0f, 120.0f, XorStr("%.0f"));
	IMGUI_TIPS("右键(推/抓)角度。");

	ImGui::Separator();
	ImGui::SliderFloat(XorStr("Auto Melee Range"), &m_fExtraMeleeRange, 0.0f, 100.0f, XorStr("%.0f"));
	IMGUI_TIPS("近战武器攻击范围预测。");

	ImGui::SliderFloat(XorStr("Auto Shove Range"), &m_fExtraShoveRange, 0.0f, 100.0f, XorStr("%.0f"));
	IMGUI_TIPS("右键(推/抓)范围预测。");

	ImGui::Checkbox(XorStr("Debug Info"), &m_bDebug);

	ImGui::TreePop();
}

void CKnifeBot::OnConfigLoading(CProfile& cfg)
{
	const std::string mainKeys = XorStr("Knifebot");
	
	m_bAutoFire = cfg.GetBoolean(mainKeys, XorStr("knifebot_melee"), m_bAutoFire);
	m_bAutoShove = cfg.GetBoolean(mainKeys, XorStr("knifebot_shove"), m_bAutoShove);
	m_bFastMelee = cfg.GetBoolean(mainKeys, XorStr("knifebot_fastmelee"), m_bFastMelee);
	m_fExtraMeleeRange = cfg.GetFloat(mainKeys, XorStr("knifebot_melee_range"), m_fExtraMeleeRange);
	m_fExtraShoveRange = cfg.GetFloat(mainKeys, XorStr("knifebot_shove_range"), m_fExtraShoveRange);
	m_bVelExt = cfg.GetBoolean(mainKeys, XorStr("knifebot_velext"), m_bVelExt);
	m_bForwardtrack = cfg.GetBoolean(mainKeys, XorStr("knifebot_forwardtrack"), m_bForwardtrack);
	m_fMeleeFOV = cfg.GetFloat(mainKeys, XorStr("knifebot_melee_fov"), m_fMeleeFOV);
	m_fShoveFOV = cfg.GetFloat(mainKeys, XorStr("knifebot_shove_fov"), m_fShoveFOV);
	m_bVisualOnly = cfg.GetBoolean(mainKeys, XorStr("knifebot_visual"), m_bVisualOnly);
}

void CKnifeBot::OnConfigSave(CProfile& cfg)
{
	const std::string mainKeys = XorStr("Knifebot");
	
	cfg.SetValue(mainKeys, XorStr("knifebot_melee"), m_bAutoFire);
	cfg.SetValue(mainKeys, XorStr("knifebot_shove"), m_bAutoShove);
	cfg.SetValue(mainKeys, XorStr("knifebot_fastmelee"), m_bFastMelee);
	cfg.SetValue(mainKeys, XorStr("knifebot_melee_range"), m_fExtraMeleeRange);
	cfg.SetValue(mainKeys, XorStr("knifebot_shove_range"), m_fExtraShoveRange);
	cfg.SetValue(mainKeys, XorStr("knifebot_velext"), m_bVelExt);
	cfg.SetValue(mainKeys, XorStr("knifebot_forwardtrack"), m_bForwardtrack);
	cfg.SetValue(mainKeys, XorStr("knifebot_melee_fov"), m_fMeleeFOV);
	cfg.SetValue(mainKeys, XorStr("knifebot_shove_fov"), m_fShoveFOV);
	cfg.SetValue(mainKeys, XorStr("knifebot_visual"), m_bVisualOnly);
}

void CKnifeBot::OnEnginePaint(PaintMode_t)
{
	if (!m_bDebug)
		return;

	int width = 0, height = 0;
	g_pInterface->Engine->GetScreenSize(width, height);

	g_pDrawing->DrawText(width/2, height/2, CDrawing::WHITE, true, XorStr("m_iSequence=%d"), m_iSequence);
}

bool CKnifeBot::RunFastMelee(CUserCmd* cmd, int weaponId, float nextAttack, float serverTime)
{
	switch (m_eMeleeStage)
	{
		case FMS_None:
		{
			if (weaponId == Weapon_Melee && nextAttack <= serverTime)
			{
				cmd->buttons |= IN_ATTACK;
				m_eMeleeStage = FMS_Primary;
				return true;
			}
			else if (m_iMeleeTick > 0)
				m_iMeleeTick = 0;

			break;
		}
		case FMS_Primary:
		{
			if (weaponId == Weapon_Melee && nextAttack > serverTime)
			{
				g_pInterface->Engine->ClientCmd_Unrestricted(XorStr("lastinv"));
				m_eMeleeStage = FMS_Secondary;
				return true;
			}
			else
				m_iMeleeTick += 1;

			break;
		}
		case FMS_Secondary:
		{
			if (weaponId != Weapon_Melee)
			{
				g_pInterface->Engine->ClientCmd_Unrestricted(XorStr("lastinv"));
				m_eMeleeStage = FMS_None;
				return true;
			}
			else
				m_iMeleeTick += 1;

			break;
		}
	}

	if (m_iMeleeTick >= 10)
	{
		m_iMeleeTick = 0;
		m_eMeleeStage = FMS_None;
	}

	return (m_iMeleeTick > 0);
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

bool CKnifeBot::HasEnemyVisible(CBasePlayer* entity, const Vector& position)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr)
		return false;

	Ray_t ray;
	CTriggerTraceFilter filter;
	ray.Init(local->GetEyePosition(), position);
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

	return (trace.m_pEnt == entity || trace.fraction > 0.97f);
}

bool CKnifeBot::IsShoveReady(CBasePlayer * player, CBaseWeapon * weapon)
{
	if (player->GetTeam() == 3)
		return true;
	
	m_iSequence = player->GetNetProp<WORD>(XorStr("DT_BaseAnimating"), XorStr("m_nSequence"));
	if (m_iSequence == ANIM_GRENADE_THORWING || m_iSequence == ANIM_GRENADE_THROWED)
		return false;

	return player->IsReadyToShove();
}

bool CKnifeBot::CheckMeleeAttack(const QAngle& myEyeAngles)
{
	m_bCanMeleeAttack = false;
	m_bCanShoveAttack = false;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return false;

	static ConVar* cvShoveRange = g_pInterface->Cvar->FindVar(XorStr("z_gun_range"));
	static ConVar* cvClawRange = g_pInterface->Cvar->FindVar(XorStr("claw_range"));
	static ConVar* cvMeleeRange = g_pInterface->Cvar->FindVar(XorStr("melee_range"));
	static ConVar* cvShoveCharger = g_pInterface->Cvar->FindVar(XorStr("z_charger_allow_shove"));

	int team = local->GetTeam();
	bool canShoveCharger = (cvShoveCharger->GetInt() > 0);
	int maxEntity = g_pInterface->EntList->GetHighestEntityIndex();
	float swingRange = (team == 3 ? cvClawRange->GetFloat() : cvShoveRange->GetFloat());
	float meleeRange = cvMeleeRange->GetFloat();

	Vector myEyePosition = local->GetEyePosition();
	if (m_bVelExt)
		myEyePosition = math::VelocityExtrapolate(myEyePosition, local->GetVelocity(), m_bForwardtrack);

	swingRange += m_fExtraShoveRange;
	meleeRange += m_fExtraMeleeRange;

	for (int i = 1; i <= maxEntity; ++i)
	{
		CBasePlayer* entity = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(i));
		if (entity == nullptr || entity == local || !entity->IsAlive() || entity->GetTeam() == team ||
			entity->GetClassID() == ET_TankRock)
			continue;

		if (entity->IsPlayer() && entity->IsGhost())
			continue;

		int targetTeam = entity->GetTeam();
		if (targetTeam == 4 || (targetTeam == 3 && entity->IsGhost()))
			continue;

		Vector aimPosition = entity->GetHeadOrigin();
		if (m_bVisualOnly && !HasEnemyVisible(entity, aimPosition))
			continue;

		if (m_bVelExt)
			aimPosition = math::VelocityExtrapolate(aimPosition, entity->GetVelocity(), m_bForwardtrack);

		int classId = entity->GetClassID();
		float dist = math::GetVectorDistance(myEyePosition, aimPosition, true);
		float fov = math::GetAnglesFieldOfView(myEyeAngles, math::CalculateAim(myEyePosition, aimPosition));

		if (!m_bCanMeleeAttack &&
			dist <= meleeRange && fov <= m_fMeleeFOV &&
			classId != ET_BOOMER && (classId != ET_WITCH || entity->GetNetProp<float>(XorStr("DT_Witch"), XorStr("m_rage")) >= 1.0f))
		{
			// 近战武器攻击 (左键)
			m_bCanMeleeAttack = true;
		}

		if (!m_bCanShoveAttack &&
			dist <= swingRange && fov <= m_fShoveFOV &&
			classId != ET_TANK && classId != ET_WITCH &&
			(classId != ET_CHARGER || canShoveCharger) &&
			(!entity->IsPlayer() || !entity->IsStaggering()))
		{
			// 推 (右键)
			m_bCanShoveAttack = true;
		}

		if (m_bCanMeleeAttack && m_bCanShoveAttack)
			break;
	}

	return (m_bCanMeleeAttack || m_bCanShoveAttack);
}
