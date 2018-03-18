#include "baseplayer.h"
#include "../interfaces.h"
#include "../hook.h"
#include "convar.h"
#include "../indexes.h"

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
