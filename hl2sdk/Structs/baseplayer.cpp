#include "baseplayer.h"
#include "../interfaces.h"
#include "../hook.h"
#include "convar.h"
#include "../indexes.h"

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

Vector CBasePlayer::GetEyePosition()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_vecViewOffset[0]"));
	Assert_NetProp(offset);
	return (GetAbsOrigin() + DECL_NETPROP_GET(Vector));
}

QAngle CBasePlayer::GetEyeAngles()
{
	static int offset = GetNetPropOffset(XorStr("DT_CSPlayer"), XorStr("m_angEyeAngles[0]"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(QAngle);
}

float CBasePlayer::GetFriction()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_flFriction"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(float);
}

int CBasePlayer::GetTeam()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_iTeamNum"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(byte);
}

Vector CBasePlayer::GetVelocity()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_vecVelocity[0]"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(Vector);
}

int CBasePlayer::GetHealth()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_iHealth"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(int);
}

int CBasePlayer::GetTempHealth()
{
	static int bufferOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_healthBuffer"));
	static int timeOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_healthBufferTime"));
	static ConVar* decayRate = g_pClientInterface->Cvar->FindVar(XorStr("pain_pills_decay_rate"));
	int amount = static_cast<int>(ceil(DECL_NETPROP_GET_EX(bufferOffset, float) -
		((g_pClientPrediction->GetServerTime() - DECL_NETPROP_GET_EX(timeOffset, float)) *
		decayRate->GetFloat())));

	return max(amount, 0);
}

CBaseWeapon * CBasePlayer::GetActiveWeapon()
{
	static int offset = GetNetPropOffset(XorStr("DT_BaseCombatCharacter"), XorStr("m_hActiveWeapon"));
	Assert_NetProp(offset);

	CBaseHandle handle = DECL_NETPROP_GET(CBaseHandle);
	if (!handle.IsValid())
		return nullptr;

	return reinterpret_cast<CBaseWeapon*>(g_pClientInterface->EntList->GetClientEntityFromHandle(handle));
}

int CBasePlayer::GetCrosshairID()
{
	return DECL_NETPROP_GET_EX(indexes::GetCrosshairsId, int);
}

bool CBasePlayer::IsGhost()
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_isGhost"));
	Assert_NetProp(offset);
	return (DECL_NETPROP_GET(byte) != 0);
}

int CBasePlayer::GetAmmo(int ammoType)
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_iAmmo"));
	Assert_NetProp(offset);
	return *reinterpret_cast<WORD*>(reinterpret_cast<DWORD>(this) + offset + (ammoType * 4));
}

CBasePlayer * CBasePlayer::GetAttacker()
{
	static int tongueOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_tongueOwner"));
	static int rideOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_jockeyAttacker"));
	static int pounceOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_pounceAttacker"));
	static int pummelOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_pummelAttacker"));
	static int carryOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_carryAttacker"));
	Assert_NetProp(tongueOffset);
	Assert_NetProp(rideOffset);
	Assert_NetProp(pounceOffset);
	Assert_NetProp(pummelOffset);
	Assert_NetProp(carryOffset);

	CBaseHandle handle = DECL_NETPROP_GET_EX(tongueOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(rideOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(pounceOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(pummelOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(carryOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;

	return nullptr;

finish_check_handle:
	return reinterpret_cast<CBasePlayer*>(g_pClientInterface->EntList->GetClientEntityFromHandle(handle));
}

CBasePlayer * CBasePlayer::GetVictim()
{
	static int tongueOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_tongueVictim"));
	static int rideOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_jockeyVictim"));
	static int pounceOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_pounceVictim"));
	static int pummelOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_pummelVictim"));
	static int carryOffset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_carryVictim"));
	Assert_NetProp(tongueOffset);
	Assert_NetProp(rideOffset);
	Assert_NetProp(pounceOffset);
	Assert_NetProp(pummelOffset);
	Assert_NetProp(carryOffset);

	CBaseHandle handle = DECL_NETPROP_GET_EX(tongueOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(rideOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(pounceOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(pummelOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;
	handle = DECL_NETPROP_GET_EX(carryOffset, CBaseHandle);
	if (handle.IsValid())
		goto finish_check_handle;

	return nullptr;

finish_check_handle:
	return reinterpret_cast<CBasePlayer*>(g_pClientInterface->EntList->GetClientEntityFromHandle(handle));
}

bool CBasePlayer::IsOnGround()
{
	return (GetGroundEntity() != nullptr && (GetFlags() & FL_ONGROUND));
}

Vector CBasePlayer::GetHeadOrigin()
{
	int classId = GetClassID();

	if (classId == ET_SURVIVORBOT || classId == ET_CTERRORPLAYER || classId == ET_TANK ||
		classId == ET_WITCH || classId == ET_SMOKER || classId == ET_BOOMER || classId == ET_HUNTER)
		return GetHitboxOrigin(HITBOX_PLAYER);

	if (classId == ET_JOCKEY || classId == ET_SPITTER)
		return GetHitboxOrigin(HITBOX_JOCKEY);

	if (classId == ET_CHARGER)
		return GetHitboxOrigin(HITBOX_CHARGER);

	if (classId == ET_INFECTED)
		return GetHitboxOrigin(HITBOX_COMMON);

	return INVALID_VECTOR;
}

bool CBasePlayer::IsAlive()
{
	if (IsDormant())
		return false;
	
	static int lifeOffset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_lifeState"));
	static int solidOffset = GetNetPropOffset(XorStr("DT_BaseCombatCharacter"), XorStr("m_usSolidFlags")) + GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_Local"));
	static int sequenceOffset = GetNetPropOffset(XorStr("DT_BaseAnimating"), XorStr("m_nSequence"));
	static int burnOffset = GetNetPropOffset(XorStr("DT_Infected"), XorStr("m_bIsBurning"));
	Assert_NetProp(lifeOffset);
	Assert_NetProp(solidOffset);
	Assert_NetProp(sequenceOffset);
	Assert_NetProp(burnOffset);

	int entityType = GetClassID();
	if (entityType == ET_INVALID || entityType == ET_WORLD)
		return false;

	// 生还者
	if (entityType == ET_SURVIVORBOT || entityType == ET_CTERRORPLAYER)
	{
		// 生还者即使是 0 血也可以活着的 (因为还有虚血)
		return (DECL_NETPROP_GET_EX(lifeOffset, byte) == LIFE_ALIVE);
	}

	// 特感
	if (entityType == ET_BOOMER || entityType == ET_HUNTER || entityType == ET_SMOKER ||
		entityType == ET_SPITTER || entityType == ET_JOCKEY || entityType == ET_CHARGER || entityType == ET_TANK)
	{
		if ((DECL_NETPROP_GET_EX(lifeOffset, byte) != LIFE_ALIVE) || GetHealth() <= 0 || IsGhost())
			return false;

		if (entityType == ET_TANK)
		{
			// Tank 在死亡前会有个濒死状态 (就是原地动不了但是还没有变尸体)
			if (DECL_NETPROP_GET_EX(sequenceOffset, WORD) > 70 || IsIncapacitated())
				return false;
		}

		return true;
	}

	// 普感
	if (entityType == ET_WITCH || entityType == ET_INFECTED)
	{
		if ((DECL_NETPROP_GET_EX(solidOffset, int) & SF_NOT_SOLID) ||
			DECL_NETPROP_GET_EX(sequenceOffset, WORD) > 305)
			return false;

		if (entityType == ET_INFECTED)
		{
			if (DECL_NETPROP_GET_EX(burnOffset, byte) != 0)
				return false;
		}

		return true;
	}

	return false;
}

int CBasePlayer::GetMoveType()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("movetype"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(int);
}

Vector& CBasePlayer::GetPunch()
{
	return DECL_NETPROP_GET_EX(indexes::GetPunch, Vector);
}

int& CBasePlayer::GetTickBase()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_nTickBase"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(int);
}

int& CBasePlayer::GetFlags()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_fFlags"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(int);
}

CBaseEntity * CBasePlayer::GetGroundEntity()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_hGroundEntity"));
	Assert_NetProp(offset);

	CBaseHandle handle = DECL_NETPROP_GET(CBaseHandle);
	if (!handle.IsValid())
		return nullptr;

	return reinterpret_cast<CBaseWeapon*>(g_pClientInterface->EntList->GetClientEntityFromHandle(handle));
}

int CBasePlayer::GetWaterLevel()
{
	static int offset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_nWaterLevel"));
	Assert_NetProp(offset);
	return DECL_NETPROP_GET(byte);
}

std::string CBasePlayer::GetName()
{
	player_info_t info;
	if (!g_pClientInterface->Engine->GetPlayerInfo(GetIndex(), &info))
		return "";

	return info.name;
}

std::pair<Vector, Vector> CBasePlayer::GetBoundingBox()
{
	static int collOffset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_Collision"));
	static int minOffset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_vecMins"));
	static int maxOffset = GetNetPropOffset(XorStr("DT_BasePlayer"), XorStr("m_vecMaxs"));
	
	Vector origin = GetAbsOrigin();
	return std::make_pair(origin + DECL_NETPROP_GET_EX(collOffset + minOffset, Vector),
		origin + DECL_NETPROP_GET_EX(collOffset + maxOffset, Vector));
}

bool CBasePlayer::IsIncapacitated()
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_isIncapacitated"));
	Assert_NetProp(offset);
	return (DECL_NETPROP_GET(byte) != 0);
}

bool CBasePlayer::IsHangingFromLedge()
{
	static int offset = GetNetPropOffset(XorStr("DT_TerrorPlayer"), XorStr("m_isHangingFromLedge"));
	Assert_NetProp(offset);
	return (DECL_NETPROP_GET(byte) != 0);
}
