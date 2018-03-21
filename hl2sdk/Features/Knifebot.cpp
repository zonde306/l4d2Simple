#include "Knifebot.h"
#include "../Utils/math.h"
#include "../hook.h"

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

CKnifeBot::CKnifeBot() : CBaseFeatures::CBaseFeatures()
{
}

CKnifeBot::~CKnifeBot()
{
	CBaseFeatures::~CBaseFeatures();
}

void CKnifeBot::OnCreateMove(CUserCmd * cmd, bool *)
{
	CBasePlayer* player = g_pClientPrediction->GetLocalPlayer();
	if (player == nullptr || !player->IsAlive())
		return;

	CBaseWeapon* weapon = player->GetActiveWeapon();
	if (weapon == nullptr)
		return;
	
	int weaponId = weapon->GetWeaponID();
	float nextAttack = weapon->GetNextPrimary();
	float serverTime = g_pClientPrediction->GetServerTime();

	if (m_bFastMelee)
	{
		if (RunFastMelee(cmd, weaponId, nextAttack, serverTime))
			return;
	}

	CanMeleeAttack(cmd->viewangles);

	if (m_bAutoFire && m_bCanMeleeAttack)
	{
		if (weaponId == Weapon_Melee && nextAttack <= serverTime)
		{
			cmd->buttons |= IN_ATTACK;
			return;
		}
	}

	if (m_bAutoShove && m_bCanShoveAttack)
	{
		nextAttack = weapon->GetSecondry();
		if (nextAttack <= serverTime)
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
	ImGui::Checkbox(XorStr("Auto Shove"), &m_bAutoShove);
	ImGui::Checkbox(XorStr("Melee Faster"), &m_bFastMelee);

	ImGui::Separator();
	ImGui::SliderFloat(XorStr("Auto Melee Range"), &m_fExtraMeleeRange, 0.0f, 50.0f, XorStr("%.0f"));
	ImGui::SliderFloat(XorStr("Auto Shove Range"), &m_fExtraShoveRange, 0.0f, 50.0f, XorStr("%.0f"));

	ImGui::TreePop();
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
				m_eMeleeStage = FMS_Primary;
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

	return false;
}

bool CKnifeBot::HasEnemyVisible(CBasePlayer* entity, const Vector& position)
{
	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr)
		return false;

	Ray_t ray;
	CTraceFilter filter;
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

bool CKnifeBot::CanMeleeAttack(const QAngle& myEyeAngles)
{
	m_bCanMeleeAttack = false;
	m_bCanShoveAttack = false;

	CBasePlayer* local = g_pClientPrediction->GetLocalPlayer();
	if (local == nullptr || !local->IsAlive())
		return false;

	static ConVar* cvShovRange = g_pInterface->Cvar->FindVar(XorStr("z_gun_range"));
	static ConVar* cvClawRange = g_pInterface->Cvar->FindVar(XorStr("claw_range"));
	static ConVar* cvMeleeRange = g_pInterface->Cvar->FindVar(XorStr("melee_range"));

	int maxEntity = g_pInterface->Engine->GetMaxClients(), i = 0;
	float swingRange = (local->GetTeam() == 3 ? cvClawRange->GetFloat() : cvShovRange->GetFloat());
	float meleeRange = cvMeleeRange->GetFloat();
	Vector myEyePosition = local->GetEyePosition();

	swingRange += m_fExtraShoveRange;
	meleeRange += m_fExtraMeleeRange;

	auto _CheckEntity = [&](int index) -> bool
	{
		CBasePlayer* player = reinterpret_cast<CBasePlayer*>(g_pInterface->EntList->GetClientEntity(index));
		if (player == nullptr || player == local || !player->IsAlive())
			return false;

		Vector aimPosition = player->GetHeadOrigin();
		if (!HasEnemyVisible(player, aimPosition))
			return false;

		int classId = player->GetClassID();
		float dist = math::GetVectorDistance(myEyePosition, aimPosition, true);
		float fov = math::GetAnglesFieldOfView(myEyeAngles, math::CalculateAim(myEyePosition, aimPosition));

		if (!m_bCanMeleeAttack &&
			dist < meleeRange && fov < meleeRange &&
			classId != ET_BOOMER && classId != ET_WITCH)
		{
			// 近战武器攻击 (左键)
			m_bCanMeleeAttack = true;
		}

		if (!m_bCanShoveAttack &&
			dist < swingRange && fov < swingRange &&
			classId != ET_TANK && classId != ET_WITCH && classId != ET_CHARGER)
		{
			// 推 (右键)
			// TODO: 牛在 z_charger_allow_shove 设置为 1 时可以被推
			m_bCanShoveAttack = true;
		}

		return (m_bCanMeleeAttack && m_bCanShoveAttack);
	};

	for (i = 1; i <= maxEntity; ++i)
	{
		if(_CheckEntity(i))
			return true;
	}

	if (m_bCanMeleeAttack && m_bCanShoveAttack)
		return true;

	i = maxEntity + 1;
	maxEntity = g_pInterface->EntList->GetHighestEntityIndex();
	for (; i <= maxEntity; ++i)
	{
		if (_CheckEntity(i))
			return true;
	}

	return (m_bCanMeleeAttack && m_bCanShoveAttack);
}
